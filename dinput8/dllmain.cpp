//-----------------------------------------------------------------------------
// dllmain.cpp
// 
// Releases:
//     1.0  - Initial release
//     1.1  - "FPS Unlock" & "Aspect Correction" improvements, "Launcher Check",
//            "Skip Logo&Legals" & "FPS Lock" added, "Force Resolution" bugfix.
//     1.1a - Added "Force DX11" option, fixed a bug with force resolution.
//     1.1b - Added support for 2675, fixed dinput8.dll not found on 32-bit
//            systems, made WinAPI error messages more verbose.
// 
// Copyright (c) 2021-2024 Vaana
//-----------------------------------------------------------------------------

#include "config.h"
#include "patching.h"
#include "fix_fps.h"
#include "fix_resolution.h"
#include "fix_aspect.h"
#include "opt_borderless_window.h"
#include "opt_skip_logos_and_legals.h"
#include "opt_version.h"

const TCHAR configFile[] = TEXT("lanvp.ini");

void Init()
{
	MessageBoxA(NULL, "A", "A", MB_OK);

	// Load or create the config
	Options options;
	if (!LoadConfig(configFile, options))
	{
		MakeConfig(configFile);
	}

	RegisterPatch_Version();

	// Register requested patches
	if (options.fps_unlock)
	{
		RegisterPatch_Framerate();
	}

	if (options.resolution_unlock)
	{
		RegisterPatch_Resolution();
	}

	if (options.aspect_correction)
	{
		RegisterPatch_Aspect();
	}

	if (options.fps_limit > 0.0)
	{
		minFrameTime = 1.0 / options.fps_limit;
	}

	if (options.borderless_window)
	{
		RegisterPatch_BorderlessWindow();
	}

	if (options.skip_logos_and_legals)
	{
		RegisterPatch_SkipLogosAndLegals();
	}

	// Execute
	DoPatches();
};

static LONG isInit = 0;

extern "C" __declspec(dllexport) void InitializeASI()
{
	if (_InterlockedCompareExchange(&isInit, 1, 0) != 0)
	{
		return;
	}

	Init();
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:	InitializeASI();	break;
		case DLL_PROCESS_DETACH:						break;
		default:										break;
	}
	return TRUE;
}
