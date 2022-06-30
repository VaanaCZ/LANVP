//-----------------------------------------------------------------------------
// lanPatch.h
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "lanConstants.h"

#pragma once

//
// Main stuff
//

#define PATCH_INSTRUCTION(address, instruction) Patcher::WriteMemory(address, &instruction, sizeof(instruction), true);
#define PATCH_MEMORY(address, memory)			Patcher::WriteMemory(address, &memory, sizeof(memory), false);

class Patcher
{
public:

	static bool			Init();

	static void			PatchFramerate();				// Total FPS unlock
	static void			PatchAspect();					// Aspect & FOV fix

	static void			SkipLauncherCheck();			// Allows the game to start without LanLauncher
	static void			SkipLogoAndLegals();			// Skip logos and legal
	static void			ForceDX11();					// Forces DirectX 11 mode
	static void			ForceResolution(int, int);		// Forces a custom resolution
	static void			ForceBorderless();				// Borderless window

	static inline void	SetFPSLock(int limit)				{ fpsLimit = limit; }
	static inline void	SetFOVMultiplier(float multiplier)	{ fovMultiplier = multiplier; }

private:

	// Patching methods/constants
	static inline Address*			renderer = nullptr;

	static NO_SECURITY_CHECKS BOOL APIENTRY HookVerQueryValue(LPCVOID, LPCSTR, LPVOID*, PUINT);
	static inline Address hookVerQueryValueAddress = (Address)&HookVerQueryValue;

	// Framerate patch
	static NO_SECURITY_CHECKS char HookFrame(int);

	static inline LARGE_INTEGER	lastTime, timeFrequency;
	static inline bool			firstFrame = true;
	static inline float*		rendererFps;
	static inline double		carBraking = LAN_DEFAULT_BRAKING_CONSTANT;

	static inline int			fpsLimit		= 0;
	static inline LONGLONG		minFrameTime	= 0;

	class UIFullMap
	{
	public:
		NO_SECURITY_CHECKS char UpdateMap(float, int);
		typedef char (UIFullMap::* UpdateMap_t)(float, int);
	};

	static inline UIFullMap::UpdateMap_t updateMapAddr = &UIFullMap::UpdateMap;
	static inline UIFullMap::UpdateMap_t originalMapAddr;


	// temp

	class Actor_ObjectInspection
	{
	public:
		NO_SECURITY_CHECKS char Inspect1(float);
		typedef char (Actor_ObjectInspection::* Inspect1_t)(float);

		NO_SECURITY_CHECKS char Inspect2(float);
		typedef char (Actor_ObjectInspection::* Inspect2_t)(float);
	};

	static inline Actor_ObjectInspection::Inspect1_t inspect1Addr = &Actor_ObjectInspection::Inspect1;
	static inline Actor_ObjectInspection::Inspect1_t originalInspect1Addr;

	static inline Actor_ObjectInspection::Inspect2_t inspect2Addr = &Actor_ObjectInspection::Inspect2;
	static inline Actor_ObjectInspection::Inspect2_t originalInspect2Addr;

	
	class CarDynamicsInstance
	{
	public:
		NO_SECURITY_CHECKS int E576A0(float);
		NO_SECURITY_CHECKS int E4A020(float);
		NO_SECURITY_CHECKS int E4AFE0(float);
		NO_SECURITY_CHECKS void E4C2F0(float);
		NO_SECURITY_CHECKS void E490B0(float);
		NO_SECURITY_CHECKS int E4F3A0(float);
		NO_SECURITY_CHECKS int E524B0(float, char, char);
		NO_SECURITY_CHECKS void E49830(float, float*);

		typedef int (CarDynamicsInstance::* sub1_t)(float);
		typedef void (CarDynamicsInstance::* sub2_t)(float);
		typedef int (CarDynamicsInstance::* sub3_t)(float, char, char);
		typedef void (CarDynamicsInstance::* sub4_t)(float, float*);
	};

	static inline CarDynamicsInstance::sub1_t addressE576A0 = &CarDynamicsInstance::E576A0;
	static inline CarDynamicsInstance::sub1_t addressE4A020 = &CarDynamicsInstance::E4A020;
	static inline CarDynamicsInstance::sub1_t addressE4AFE0 = &CarDynamicsInstance::E4AFE0;
	static inline CarDynamicsInstance::sub2_t addressE4C2F0 = &CarDynamicsInstance::E4C2F0;
	static inline CarDynamicsInstance::sub2_t addressE490B0 = &CarDynamicsInstance::E490B0;
	static inline CarDynamicsInstance::sub1_t addressE4F3A0 = &CarDynamicsInstance::E4F3A0;
	static inline CarDynamicsInstance::sub3_t addressE524B0 = &CarDynamicsInstance::E524B0;
	static inline CarDynamicsInstance::sub4_t addressE49830 = &CarDynamicsInstance::E49830;

	static inline CarDynamicsInstance::sub1_t originalAddressE576A0;
	static inline CarDynamicsInstance::sub1_t originalAddressE4A020;
	static inline CarDynamicsInstance::sub1_t originalAddressE4AFE0;
	static inline CarDynamicsInstance::sub2_t originalAddressE4C2F0;
	static inline CarDynamicsInstance::sub2_t originalAddressE490B0;
	static inline CarDynamicsInstance::sub1_t originalAddressE4F3A0;
	static inline CarDynamicsInstance::sub3_t originalAddressE524B0;
	static inline CarDynamicsInstance::sub4_t originalAddressE49830;

		
	// end temp


	// Aspect patch
	class CameraRelativeLookAtModifier
	{
	public:
		NO_SECURITY_CHECKS void UpdateFov(void*);
		typedef void (CameraRelativeLookAtModifier::*UpdateFov_t)(void*);
	};

	static inline CameraRelativeLookAtModifier::UpdateFov_t updateFovAddr = &CameraRelativeLookAtModifier::UpdateFov;
	static inline CameraRelativeLookAtModifier::UpdateFov_t originalFovAddr;

	static NO_SECURITY_CHECKS int __cdecl HookAtoi(const char*);
	static inline Address	hookAtoiAddress = (Address)&HookAtoi;

	static inline Address	interfaceWidthAddr;
	static inline Address	interfaceHeightAddr;

	static inline float		aspectMultiplier	= 1.0f;
	static inline float		fovMultiplier		= 1.0f;

	// Force resolution
	static NO_SECURITY_CHECKS int __cdecl HookStrtol(const char*, char**, int);
	static inline Address	hookStrtolAddress = (Address)&HookStrtol;
	static inline bool		strtolCalled = false;

	static inline int	width;
	static inline int	height;

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

