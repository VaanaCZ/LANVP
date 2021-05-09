//-----------------------------------------------------------------------------
// lanPatch.cpp
// 
// Releases:
//     1.0 - Initial release
//
// Copyright (c) 2021 Václav AKA Vaana
//-----------------------------------------------------------------------------

#include "lanPatch.h"

bool Patcher::Init()
{
	#define ERROR(msg)	MessageBoxW(NULL, msg, L"[V-Patch] Error while initializing patch.", MB_OK);

	//
	// Retrieve all needed handles and values
	//
	hProcess	= GetCurrentProcess();
	hModule		= GetModuleHandleW(NULL);

	if (!hModule)
	{
		ERROR(L"Failed to obtain module handle. The fixes will not be applied!");
		return false;
	}

	// Headers
	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		ERROR(L"Failed to read module header (IMAGE_DOS_HEADER). The fixes will not be applied!");
		return false;
	}

	IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)((char*)dosHeader + dosHeader->e_lfanew);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		ERROR(L"Failed to read module header (IMAGE_NT_HEADERS). The fixes will not be applied!");
		return false;
	}

	linkTimestamp = ntHeaders->FileHeader.TimeDateStamp;

	//
	// Do a version check
	//
	switch (linkTimestamp)
	{
	case LAN_TIMESTAMP_2617R:	lanVersion = LAN_VERSION_2617R;	 break;
	case 0: // The old Steam version has a 0 timestamp, so we assume it's 2617
	case LAN_TIMESTAMP_2617S:	lanVersion = LAN_VERSION_2617S;	 break;
	case LAN_TIMESTAMP_2663R:	lanVersion = LAN_VERSION_2663R;	 break;
	case LAN_TIMESTAMP_2663S:	lanVersion = LAN_VERSION_2663S;	 break;

	default:
		ERROR(L"Unrecognized L.A. Noire version, please make sure you are running a supported build. The fixes will not be applied!");
		return false;
	}

	renderer = (Address*)ResolveAddress(OFFSET_POINTER_RENDERER);

	return true;
}

//-------------------------------------------------------------

#define ERROR_ADDRESS(msg, addr)													\
	wchar_t errMsg[MAX_PATH];														\
	swprintf_s(errMsg, MAX_PATH, msg, addr);										\
	MessageBoxW(NULL, errMsg, L"[V-Patch] Error while patching memory.", MB_OK);

bool Patcher::WriteMemory(Address address, void* in, unsigned long size, bool flushInstructionCache)
{
	//
	// Some basic error checking
	//
	if (address == 0  || size == 0)
	{
		ERROR_ADDRESS(L"Failed to write memory at address 0x%08x. Address and size must not be 0!", address);
		return false;
	}

	//
	// Query memory range
	//
	MEMORY_BASIC_INFORMATION mbi;
	SIZE_T mbiSize = sizeof(MEMORY_BASIC_INFORMATION);

	if (VirtualQuery((void*)address, &mbi, mbiSize) != mbiSize)
	{
		ERROR_ADDRESS(L"Failed to write memory at address 0x%08x. Could not query virtual address!", address);
		return false;
	}

	//
	// Remove protection for memory region
	//
	DWORD oldProtection = 0;
	bool isProtected = false;

	if (!(mbi.Protect & PAGE_EXECUTE_READWRITE))
	{
		if (VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			isProtected = true;
		}
		else
		{
			ERROR_ADDRESS(L"Failed to write memory at address 0x%08x. Could not remove protection on virtual address!", address);
			return false;
		}
	}

	//
	// Copy memory
	//
	memcpy((void*)address, in, size);

	//
	// Restore protection
	//
	if (isProtected)
	{
		if (!VirtualProtect((void*)address, size, oldProtection, &oldProtection))
		{
			ERROR_ADDRESS(L"Failed to write memory at address 0x%08x. Could not restore protection on virtual address!", address);
			return false;
		}
	}

	//
	// Flush cache if required
	//
	if (flushInstructionCache && isProtected)
	{
		if (!FlushInstructionCache(hProcess, (void*)address, size))
		{
			ERROR_ADDRESS(L"Failed to write memory at address 0x%08x. Could not flush instruction cache on virtual address!", address);
			return false;
		}
	}

	return true;
}

bool Patcher::ReadMemory(Address address, unsigned long size, void* out)
{
	//
	// Some basic error checking
	//
	if (address == 0 || size == 0 || out == 0)
	{
		ERROR_ADDRESS(L"Failed to read memory at address 0x%08x. Address, size and out must not be 0!", address);
		return false;
	}

	//
	// Query memory range
	//
	MEMORY_BASIC_INFORMATION mbi;
	SIZE_T mbiSize = sizeof(MEMORY_BASIC_INFORMATION);

	if (VirtualQuery((void*)address, &mbi, mbiSize) != mbiSize)
	{
		ERROR_ADDRESS(L"Failed to read memory at address 0x%08x. Could not query virtual address!", address);
		return false;
	}

	//
	// Remove protection for memory region
	//
	DWORD oldProtection = 0;
	bool isProtected = false;

	if (!(mbi.Protect & PAGE_EXECUTE_READWRITE))
	{
		if (VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			isProtected = true;
		}
		else
		{
			ERROR_ADDRESS(L"Failed to read memory at address 0x%08x. Could not remove protection on virtual address!", address);
			return false;
		}
	}

	//
	// Copy memory
	//
	memcpy(out, (void*)address, size);

	//
	// Restore protection
	//
	if (isProtected)
	{
		if (!VirtualProtect((void*)address, size, oldProtection, &oldProtection))
		{
			ERROR_ADDRESS(L"Failed to read memory at address 0x%08x. Could not restore protection on virtual address!", address);
			return false;
		}
	}

	return true;
}

bool Patcher::ReplaceMemory(Address address, void* in, unsigned long size, void* out, bool flushInstructionCache = true)
{
	//
	// Some basic error checking
	//
	if (address == 0 || size == 0 || out == 0)
	{
		ERROR_ADDRESS(L"Failed to replace memory at address 0x%08x. Address and size must not be 0!", address);
		return false;
	}

	//
	// Query memory range
	//
	MEMORY_BASIC_INFORMATION mbi;
	SIZE_T mbiSize = sizeof(MEMORY_BASIC_INFORMATION);

	if (VirtualQuery((void*)address, &mbi, mbiSize) != mbiSize)
	{
		ERROR_ADDRESS(L"Failed to replace memory at address 0x%08x. Could not query virtual address!", address);
		return false;
	}

	//
	// Remove protection for memory region
	//
	DWORD oldProtection = 0;
	bool isProtected = false;

	if (!(mbi.Protect & PAGE_EXECUTE_READWRITE))
	{
		if (VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			isProtected = true;
		}
		else
		{
			ERROR_ADDRESS(L"Failed to replace memory at address 0x%08x. Could not remove protection on virtual address!", address);
			return false;
		}
	}

	//
	// Copy memory
	//
	memcpy(out, (void*)address, size);
	memcpy((void*)address, in, size);

	//
	// Restore protection
	//
	if (isProtected)
	{
		if (!VirtualProtect((void*)address, size, oldProtection, &oldProtection))
		{
			ERROR_ADDRESS(L"Failed to replace memory at address 0x%08x. Could not restore protection on virtual address!", address);
			return false;
		}
	}

	//
	// Flush cache if required
	//
	if (flushInstructionCache && isProtected)
	{
		if (!FlushInstructionCache(hProcess, (void*)address, size))
		{
			ERROR_ADDRESS(L"Failed to replace memory at address 0x%08x. Could not flush instruction cache on virtual address!", address);
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------

//
// Framerate patch
// 
// - Completely unlocks the framerate.
// - Adjusts car handling to match the unlocked framerate.
//

void Patcher::PatchFramerate()
{
	//
	// Framerate sync removal
	//
	Address hookAddr = ResolveAddress(OFFSET_HOOK_FRAME);
	call frameHook = { 0xE8, (Address)&HookFrame - hookAddr - 5 };
	PATCH_INSTRUCTION(hookAddr, frameHook);

	//
	// Car handling adjustment
	// 
	// In the RLauncher version, mulsd was compiled as fmul. 
	//
	Address brakingConstantAddr = ResolveAddress(OFFSET_PATCH_CAR_BRAKE_CONSTANT);

	if (lanVersion == LAN_VERSION_2663R || lanVersion == LAN_VERSION_2617R)
	{
		fmul carHandling = { 0xDC, 0x0D, (Address)&carBreaking };
		PATCH_INSTRUCTION(brakingConstantAddr, carHandling);
	}
	else
	{
		mulsd carHandling = { 0xF2, 0x0F, 0x59, 0x05, (Address)&carBreaking };
		PATCH_INSTRUCTION(brakingConstantAddr, carHandling);
	}
	
}

NO_SECURITY_CHECKS char Patcher::HookFrame(int pointer)
{
	//
	// Here we measure the time spent rendering
	// in order to adjust the game speed. We
	// effectively implement delta timing, even
	// thought the game doesn't support it.
	//
	// BEWARE! The argument pointer is passed through
	// a register on some versions, which makes it
	// unsafe to use.
	//
	
	if (renderer != 0)
	{	
		LARGE_INTEGER currTime;
		QueryPerformanceCounter(&currTime);
	
		if (!firstFrame)
		{
			LONGLONG quadDiff = currTime.QuadPart - lastTime.QuadPart;
			
			if (quadDiff > 0)
			{
				double fps = timeFrequency.QuadPart / (double)quadDiff;

				//
				// The minimum allowed gamespeed is 29.97 FPS
				// If we go below 22 FPS, the game goes crazy
				// so let's just keep it safe.
				// 
				// We also need to multiply the FPS by 2, since
				// the game still expects it to be double the
				// actual framerate.
				//
				*rendererFps = max(fps, LAN_MINIMAL_FRAMERATE) * 2.0;

				//
				// Higher framerates have an adverse effect on
				// car handling. To "correct" this we need to
				// change the handling values according to the
				// current framerate.
				// 
				// Braking becomes much slower causing the game
				// to be unplayable on high fps. As a fix, we 
				// modify the braking speed/force constant.
				// Luckily, this constant is set to 30 by default,
				// meaning we only need to set it to the current
				// framerate to resolve the issue.
				//
				carBreaking = max(fps, LAN_DEFAULT_BRAKING_CONSTANT);

				//
				// Even with corrected braking, steering is very
				// sensitive. There was an attempt to fix this
				// by changing the steering force, but it didn't
				// scale up well (on 100+ FPS it barely turned).
				// For now, this isn't such a big deal. 
				//
			}
		}
		else
		{
			QueryPerformanceFrequency(&timeFrequency);
			rendererFps	= (float*)(*renderer + 0xE8);
			firstFrame	= false;
		}
	
		lastTime = currTime;
	}

	//
	// The game runs at double the reported framerate.
	// 
	// The first frame is when rendering is performed,
	// and the frame counter is incremented.
	// The second frame is used for game logic
	// synchronization (sleeping, not rendering).
	//
	// Based on which frame it is, the possible return
	// values of this function are:
	// 
	// 0 - synchronization frame
	// 1 - rendering frame
	// 
	// By replacing this function with a permanent
	// "return 1", every frame becomes a rendering
	// frame, which also has the effect of removing
	// the frame cap.
	//

	return 1;
}


//-------------------------------------------------------------

//
// Aspect ratio patch
// 
// - Removes black bars for aspect ratios slimmer than 16:9 (16:10, 4:3, 5:4, etc.)
//

void Patcher::PatchViewportAspect()
{
	//
	// Black bars removal
	//
	Address jmpAddr = ResolveAddress(OFFSET_PATCH_ASPECT_CHECK);
	jmpByte jmpSkip = { 0xEB, 0x47 };
	PATCH_INSTRUCTION(jmpAddr, jmpSkip);
}

//-------------------------------------------------------------

//
// Field of view patch
// 
// - Corrects FOV for unsupported aspect ratios (21:9, 16:10, 4:3, 5:4, etc.)
//

void Patcher::PatchFieldOfView()
{
	//
	// Fov recalculation
	//
	Address vTableAddr = ResolveAddress(OFFSET_HOOK_FIELD_OF_VIEW_VTABLE);
	ReplaceMemory(vTableAddr, &updateFovAddr, sizeof(updateFovAddr), (Address*)&originalFovAddr, false);
}

NO_SECURITY_CHECKS void Patcher::Camera::UpdateFov(void* copyCamera)
{
	//
	// By default, the fov value stores the horizontal
	// view angle without any kind of acknowledgement
	// of the vertical value. This causes issues when
	// the screen is wider than 16:9, since the top and
	// bottom parts of the viewport get cut off due to
	// the smaller vertical angle. 
	// 
	// Here we correct for the fact by calculating the
	// vertical value first (by assuming the screen is
	// 16:9) and then multiplying it with the current
	// aspect ratio to achieve the correct result.
	//
	// As a side-effect, the fov is a bit smaller on 
	// 4:3, but its still perfectly playable.
	//

	if (renderer != 0)
	{
		//
		// Call the original function first, to
		// get the latest fov value.
		//		
		((*this).*originalFovAddr)(copyCamera);
				
		if (firstFovUpdate)
		{
			int* viewWidth = (int*)(*renderer + 0x1D4);
			int* viewHeight = (int*)(*renderer + 0x1D8);
			viewAspect = (float)(*viewWidth) / (float)(*viewHeight);

			firstFovUpdate = false;
		}

		float* fovH = (float*)(((Address)this) + 0x64);

		//
		// Do the fov recalcuation.
		// 
		// fovV = fovH * (9/16)
		// fovH = fovV * aspect
		//
		if (fovH > 0)
		{
			float fovV = (*fovH * LAN_DEFAULT_ASPECT_INVERSE);
			*fovH = fovV * viewAspect * fovMultiplier; // Allow fov modifications
		}
	}
}

//-------------------------------------------------------------

//
// Force resolution
// 
// - Forces a selected custom resolution and refresh rate
//

void Patcher::ForceResolution(int _width, int _height, int _refreshRate)
{
	width		= _width;
	height		= _height;
	refreshRate	= _refreshRate;

	//
	// Replace atoi call
	//
	Address hookAddr = ResolveAddress(OFFSET_HOOK_RESOLUTION);
	mov atoiHook = { 0x8B, 0x35, (Address)&hookAtoiAddress };
	PATCH_INSTRUCTION(hookAddr, atoiHook);
}

NO_SECURITY_CHECKS int __cdecl Patcher::HookAtoi(const char* string)
{
	if (hookAtoiCallCount == 0)
	{
		hookAtoiCallCount++;
		return width;
	}
	else if (hookAtoiCallCount == 1)
	{
		hookAtoiCallCount++;
		return height;
	}
	else if (hookAtoiCallCount == 2)
	{
		hookAtoiCallCount = 0;
		return refreshRate;
	}
}

//-------------------------------------------------------------

//
// Borderless window
// 
// - Forces the game to run in windowed mode.
// - Makes the game window borderless.
//

void Patcher::ForceBorderless()
{
	//
	// CreateWindow hook
	//
	Address hookAddr = ResolveAddress(OFFSET_HOOK_CREATE_WINDOW);
	callPtr wndHook = { 0xFF, 0x15, (Address)&hookCreateWindowAddress };
	PATCH_INSTRUCTION(hookAddr, wndHook);

	//
	// Run in window
	//
	Address nopAddr = ResolveAddress(OFFSET_PATCH_WINDOWED);
	NopInstruction(nopAddr, 9);
}

NO_SECURITY_CHECKS HWND WINAPI Patcher::HookCreateWindow(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
														 int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
														 HINSTANCE hInstance, LPVOID lpParam)
{
	//
	// Calculate the size of client area and
	// subtract borders from size.
	//
	RECT rect = { 0, 0, LAN_DEFAULT_WIDTH, LAN_DEFAULT_HEIGHT };
	AdjustWindowRect(&rect, dwStyle, FALSE);

	int width  = nWidth  - (rect.right  - rect.left - LAN_DEFAULT_WIDTH);
	int height = nHeight - (rect.bottom - rect.top  - LAN_DEFAULT_HEIGHT);

	//
	// In order to force a borderless window, we 
	// need to specify additional parameters
	//
	dwStyle = WS_POPUP;

	return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
						   0, 0, width, height, hWndParent, hMenu,
						   hInstance, lpParam);
}

