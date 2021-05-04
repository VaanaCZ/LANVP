//-----------------------------------------------------------------------------
// lanPatch.h
// 
// Releases:
//     1.0 - Initial release
//
// Copyright (c) 2021 Václav AKA Vaana
//-----------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>

#include "lanConstants.h"

#pragma once

//
// Main stuff
//

#define PATCH_INSTRUCTION(address, instruction) Patcher::WriteMemory(address, &instruction, sizeof(instruction), true);

class Patcher
{
public:

	static bool			Init();

	static void			PatchFramerate();				// Total FPS unlock
	static void			PatchViewportAspect();			// Aspect & FOV fix
	static void			PatchFieldOfView();				// Aspect & FOV fix

	static inline void	SetFOVMultiplier(float multiplier) { fovMultiplier = multiplier; }
	static void			ForceResolution(int, int, int); // Forces a custom resolution
	static void			ForceBorderless();				// Borderless window


private:

	// Patching methods/constants
	static inline Address*			renderer = nullptr;

	// Framerate patch
	static NO_SECURITY_CHECKS char HookFrame(int);

	static inline double		frameTime;
	static inline LARGE_INTEGER	lastTime, timeFrequency;
	static inline bool			firstFrame = true;
	static inline float*		rendererFps;
	static inline double		carBreaking = LAN_DEFAULT_BRAKING_CONSTANT;

	// Field of view patch
	class Camera
	{
	public:
		NO_SECURITY_CHECKS void UpdateFov(void*);
		typedef void (Camera::*UpdateFov_t)(void*);
	};

	static inline Camera::UpdateFov_t updateFovAddr = &Camera::UpdateFov;
	static inline Camera::UpdateFov_t originalFovAddr;

	static inline bool		firstFovUpdate = true;
	static inline float		viewAspect;
	static inline float		fovMultiplier = 1.0f;

	// Force resolution
	static NO_SECURITY_CHECKS int __cdecl HookAtoi(const char*);
	static inline Address hookAtoiAddress = (Address)&HookAtoi;

	static inline int	hookAtoiCallCount = 0;

	static inline int	width;
	static inline int	height;
	static inline int	refreshRate;

	// Force borderless	
	static NO_SECURITY_CHECKS HWND WINAPI HookCreateWindow(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
	static inline Address hookCreateWindowAddress = (Address)&HookCreateWindow;

	// Helper functions
	static Address	ResolveAddress(Address offset)	{ return (Address)hModule + addresses[lanVersion][offset]; }

	static bool WriteMemory(Address, void*, unsigned long, bool);
	static bool ReadMemory(Address, unsigned long, void*);
	static bool ReplaceMemory(Address, void*, unsigned long, void*, bool);

	inline static bool NopInstruction(unsigned long addr, unsigned long nopSize)
	{
		char* nop = new char[nopSize];
		memset(nop, 0x90, nopSize);
		return WriteMemory(addr, nop, nopSize, true);
	}

	static inline HANDLE		hProcess;
	static inline HMODULE		hModule;
	static inline unsigned long	linkTimestamp;

	static inline LAN_VERSION	lanVersion;

};

