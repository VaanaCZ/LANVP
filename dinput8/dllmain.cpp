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
// Copyright (c) 2021-2022 V�clav AKA Vaana
//-----------------------------------------------------------------------------

#include <windows.h>

//#include "exports.h"
#include "lanPatch.h"
#include "config.h"

/*BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Enable DirectInput proxy
		LoadDinput8();*/

extern "C" __declspec(dllexport) BOOL InitializeASI()
{

	Config::Init();

	if (Config::options->patchEnabled &&
		Patcher::Init())
	{
		// Patches
		if (Config::options->fpsUnlock)
			Patcher::PatchFramerate();

		if (Config::options->aspectCorrection)
			Patcher::PatchAspect();

		// Options
		Patcher::SetFOVMultiplier(Config::options->fovMultiplier);
		Patcher::SetFPSLock(Config::options->fpsLock);

		int width = Config::options->forceResolutionWidth;
		int height = Config::options->forceResolutionHeight;
		if (width != 0 && height != 0)
			Patcher::ForceResolution(width, height);

		if (Config::options->forceBorderlessWindow)
			Patcher::ForceBorderless();

		if (Config::options->skipLogos)
			Patcher::SkipLogoAndLegals();

		if (Config::options->skipLauncherCheck)
			Patcher::SkipLauncherCheck();

		if (Config::options->forceDx11)
			Patcher::ForceDX11();
	}

	Config::Destroy();

	return TRUE;
};

/*		break;
	}     
	case DLL_PROCESS_DETACH: break;
	default: break;
	}
	return TRUE;
}*/

