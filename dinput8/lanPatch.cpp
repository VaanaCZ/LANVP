//-----------------------------------------------------------------------------
// lanPatch.cpp
// 
// Releases:
//     1.0  - Initial release
//     1.1  - "FPS Unlock" & "Aspect Correction" improvements, "Launcher Check",
//            "Skip Logo&Legals" & "FPS Lock" added, "Force Resolution" bugfix.
//     1.1a - Added "Force DX11" option, fixed a bug with force resolution.
//     1.1b - Added support for 2675, fixed dinput8.dll not found on 32-bit
//            systems, made WinAPI error messages more verbose.
// 
// Copyright (c) 2021-2022 Václav AKA Vaana
//-----------------------------------------------------------------------------

#include "lanPatch.h"

// todo: Check for high uptime related issues: https://github.com/CookiePLMonster/UptimeFaker
// todo: Check for problems with WinAPI: https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/application-verifier

bool Patcher::Init()
{
#define ERROR(msg)	MessageBoxW(NULL, msg, L"[V-Patch] Error while initializing patch.", MB_OK);

	//
	// Retrieve all needed handles and values
	//
	hProcess = GetCurrentProcess();
	hModule = GetModuleHandleW(NULL);

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
	case LAN_TIMESTAMP_2675S:	lanVersion = LAN_VERSION_2675S;	 break;
	case LAN_TIMESTAMP_2675R:	lanVersion = LAN_VERSION_2675R;	 break;

	default:
	{
		//
		// If the version check fails, we check the current build.
		//
		wchar_t moduleFilename[MAX_PATH];
		if (GetModuleFileNameW(hModule, moduleFilename, MAX_PATH))
		{
			DWORD  verHandle = 0;
			UINT   size = 0;
			LPBYTE lpBuffer = NULL;
			DWORD  verSize = GetFileVersionInfoSizeW(moduleFilename, &verHandle);

			if (verSize != NULL)
			{
				LPSTR verData = new char[verSize];

				if (GetFileVersionInfoW(moduleFilename, verHandle, verSize, verData) &&
					VerQueryValueW(verData, L"\\", (VOID FAR * FAR*) & lpBuffer, &size) &&
					size)
				{
					VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
					if (verInfo->dwSignature == 0xFEEF04BD)
					{
						int buildNumber = (verInfo->dwFileVersionMS >> 16) & 0xffff;

						wchar_t message[MAX_PATH];
						if (buildNumber > LAN_LATEST_BUILD)
						{
							swprintf_s(message, MAX_PATH, L"Your version (%d) is not currently supported. Please contact the author as soon as possible, so that we can implement support for it!", buildNumber);
						}
						else
						{
							swprintf_s(message, MAX_PATH, L"Your version (%d) is not supported. Please make sure you are running the latest Steam or Rockstar Launcher version!", buildNumber);
						}

						ERROR(message);
						return false;
					}
				}
				delete[] verData;
			}
		}

		ERROR(L"Unrecognized L.A. Noire version, please make sure you are running a supported build. The fixes will not be applied!");
		return false;
	}
	}

	//
	// Resolve addresses and do basic patching
	//
	renderer = (Address*)ResolveAddress(OFFSET_POINTER_RENDERER);
	
	//
	// Add version text into menu
	//
	Address hookAddr = ResolveAddress(OFFSET_HOOK_VERSION);
	call hook = { 0xE8, (Address)&HookVerQueryValue - hookAddr - 5 };
	PATCH_INSTRUCTION(hookAddr, hook);

	// todo: Force DX11 by default, if first time run.

	return true;
}

NO_SECURITY_CHECKS BOOL Patcher::HookVerQueryValue(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen)
{
	BOOL result = VerQueryValueA(pBlock, lpSubBlock, lplpBuffer, puLen);
	char* buffer = (char*)*lplpBuffer; // Memory leak! Shouldn't matter since it's called only once.

	//
	// Add V-Patch version info to build number
	//
	char printVal[] = "; V-Patch v" VPATCH_VERSION_MENU ", ";
	unsigned int newBufferSize = *puLen + strlen(printVal);
	char* newBuffer = new char[newBufferSize];

	char* cPos = strstr(buffer, ",");
	cPos[0] = '%';
	cPos[1] = 's';

	sprintf_s(newBuffer, newBufferSize, buffer, printVal); // This is a really ugly hack but it works

	*lplpBuffer = newBuffer;
	*puLen		= newBufferSize;

	return result;
}

//-----------------------------------------------------------------------------

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
	// Map mouse sensitivity adjustment
	//
	Address vTableAddr = ResolveAddress(OFFSET_HOOK_MAP_VTABLE);
	ReplaceMemory(vTableAddr, &updateMapAddr, sizeof(updateMapAddr), (Address*)&originalMapAddr, false);
	
	//
	// Car handling adjustment
	// 
	// In the RLauncher version, mulsd was compiled as fmul. 
	//
	Address brakingConstantAddr = ResolveAddress(OFFSET_PATCH_CAR_BRAKE_CONSTANT);

	if (lanVersion == LAN_VERSION_2663R || lanVersion == LAN_VERSION_2617R ||
		lanVersion == LAN_VERSION_2675R)
	{
		fmul carHandling = { 0xDC, 0x0D, (Address)&carBraking };
		PATCH_INSTRUCTION(brakingConstantAddr, carHandling);
	}
	else
	{
		mulsd carHandling = { 0xF2, 0x0F, 0x59, 0x05, (Address)&carBraking };
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

			// Frame limiter
			if (fpsLimit != 0 &&
				quadDiff < minFrameTime)
			{
				while (quadDiff < minFrameTime)
				{
					Sleep(0);

					QueryPerformanceCounter(&currTime);
					quadDiff = currTime.QuadPart - lastTime.QuadPart;
				}
			}
			
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
				carBraking = max(fps, LAN_DEFAULT_BRAKING_CONSTANT);

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

			// Frame limiter
			if (fpsLimit != 0)
			{
				minFrameTime = timeFrequency.QuadPart / fpsLimit;
			}
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

NO_SECURITY_CHECKS char Patcher::UIFullMap::UpdateMap(float a2, int a3)
{
	//
	// This just fixes the time delta to 1/60 of a second,
	// which resolves the sensitivity issue.
	//
	return ((*this).*originalMapAddr)(LAN_FRAMETIME_MENU, a3);
}

//-------------------------------------------------------------

//
// Aspect ratio patch
// 
// - Removes black bars for aspect ratios slimmer than 16:9 (16:10, 4:3, 5:4, etc.)
// - Corrects FOV for unsupported aspect ratios (21:9, 16:10, 4:3, 5:4, etc.)
// - Corrects the interface size for unsupported aspect rations
//

void Patcher::PatchAspect()
{
	//
	// Allow any aspect ratio in resolution list
	//
	Address supportedResAddr = ResolveAddress(OFFSET_PATCH_ASPECT_FILTER);
	NopInstruction(supportedResAddr, 6);

	//
	// Black bars removal
	//
	Address jmpAddr = ResolveAddress(OFFSET_PATCH_ASPECT_CROP);
	Opcode jmpSkip = 0xEB;
	PATCH_INSTRUCTION(jmpAddr, jmpSkip);

	//
	// Fov recalculation
	//
	Address vTableAddr = ResolveAddress(OFFSET_HOOK_FIELD_OF_VIEW_VTABLE);
	ReplaceMemory(vTableAddr, &updateFovAddr, sizeof(updateFovAddr), (Address*)&originalFovAddr, false);

	//
	// Interface size recalculation
	//
	Address hookAddr = ResolveAddress(OFFSET_HOOK_INTERFACE);
	callPtr hook = { 0xFF, 0x15, (Address)&hookAtoiAddress };
	PATCH_INSTRUCTION(hookAddr, hook);

	interfaceWidthAddr	= ResolveAddress(OFFSET_VALUE_INTERFACE_WIDTH);
	interfaceHeightAddr	= ResolveAddress(OFFSET_VALUE_INTERFACE_HEIGHT);
}

NO_SECURITY_CHECKS void Patcher::CameraRelativeLookAtModifier::UpdateFov(void* copyCamera)
{
	//
	// Call the original function first, to
	// get the latest fov value.
	//		
	((*this).*originalFovAddr)(copyCamera);

	//
	// By default, the fov value stores the horizontal
	// view angle without any kind of acknowledgement
	// of the vertical value. This causes issues when
	// the screen is wider than 16:9, since the top and
	// bottom parts of the viewport get cut off due to
	// the smaller vertical angle. 
	// 
	// We correct for the fact by calculating the 
	// difference between the current and default (16:9)
	// aspect ratios, and then multiplying it with the
	// current fov to achieve the right result.
	//
	// As a side-effect, the fov is a bit smaller on 
	// 4:3, but its still perfectly playable.
	//

	float* fovH = (float*)(((Address)this) + 0x64);
	*fovH *= aspectMultiplier * fovMultiplier;
}

NO_SECURITY_CHECKS int __cdecl Patcher::HookAtoi(const char* string)
{
	int* viewWidth	= (int*)(*renderer + 0x1D4);
	int* viewHeight	= (int*)(*renderer + 0x1D8);

	//
	// Here the difference between the current and
	// default (16:9) aspect ratios it calculated.
	//
	float viewAspect = (float)(*viewWidth) / (float)(*viewHeight);
	aspectMultiplier = viewAspect / LAN_DEFAULT_ASPECT;

	//
	// In order to correct for different aspect ratio, we
	// stretch the virtual viewport as needed, so that the
	// proportions are always correct.
	//
	// If the aspect is smaller than 4:3, the height must
	// be stretched instead of the width. Otherwise, there
	// will be issues with the interface being too big.
	//
	if (aspectMultiplier > 1.0f)
	{
		double interfaceWidth = 1280.0 * aspectMultiplier;
		PATCH_MEMORY(interfaceWidthAddr, interfaceWidth);
	}
	else
	{
		double interfaceHeight = 720.0 / aspectMultiplier;
		PATCH_MEMORY(interfaceHeightAddr, interfaceHeight);
	}

	//
	// At the end, just return the expected result.
	// Could just be replaced with a "return viewWidth;".
	//
	return atoi(string);
}

//-------------------------------------------------------------

//
// Skip launcher check
// 
// - Patches outh the launcher check to allow the game to start
//   without being launched through the launcher.
//

void Patcher::SkipLauncherCheck()
{
	//
	// Patch out the launcher check
	//
	Address jmpAddr = ResolveAddress(OFFSET_PATCH_LAUNCHER_CHECK);
	Opcode jmpSkip = 0xEB;
	PATCH_INSTRUCTION(jmpAddr, jmpSkip);

	// todo: Make it work for R* Launcher builds
}

//-------------------------------------------------------------

//
// Skip logo and legal
// 
// - Skips the "MovieLogo" played on startup
// - Skips the legal disclaimer screen
//

void Patcher::SkipLogoAndLegals()
{
	//
	// Patch out MovieLogo
	//
	Address logoAddr = ResolveAddress(OFFSET_PATCH_MOVIE_LOGO);
	NopInstruction(logoAddr, 14);

	//
	// Patch out the legals screen call
	//
	Address legalAddr = ResolveAddress(OFFSET_PATCH_LEGALS_SCREEN);
	Opcode legalRet = 0xC3;
	PATCH_INSTRUCTION(legalAddr, legalRet);
}

//-------------------------------------------------------------

//
// Force DX11
// 
// - Forces DirectX 11 mode regardless of the game settings.
//

void Patcher::ForceDX11()
{
	//
	// Skip the config check
	//
	Address jmpAddr = ResolveAddress(OFFSET_PATCH_DX11);
	Opcode jmpSkip = 0xEB;
	PATCH_INSTRUCTION(jmpAddr, jmpSkip);
}

//-------------------------------------------------------------

//
// Force resolution
// 
// - Forces a selected custom resolution and refresh rate
//

void Patcher::ForceResolution(int _width, int _height)
{
	width		= _width;
	height		= _height;

	//
	// Force -res argument
	//
	Address nopAddr = ResolveAddress(OFFSET_PATCH_ARGS_RESOLUTION);
	NopInstruction(nopAddr, 6);

	//
	// Strtol hook
	//
	Address callAddr = ResolveAddress(OFFSET_HOOK_RESOLUTION);
	mov callStrol = { 0x8B, 0x35, (Address)&hookStrtolAddress };
	PATCH_INSTRUCTION(callAddr, callStrol);

	//
	// Allow any resolution in the argument
	//
	Address checkAddr = ResolveAddress(OFFSET_PATCH_RESOLUTION_CHECK);
	NopInstruction(checkAddr, 2);
}

NO_SECURITY_CHECKS int __cdecl Patcher::HookStrtol(const char* String, char** EndPtr, int Radix)
{
	if (strtolCalled)
	{
		strtolCalled = false;
		return width;
	}
	else
	{
		strtolCalled = true;
		return height;
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
	Address nopAddr = ResolveAddress(OFFSET_PATCH_ARGS_WINDOWED);
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
