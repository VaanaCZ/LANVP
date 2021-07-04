//-----------------------------------------------------------------------------
// dllmain.cpp
// 
// Releases:
//     1.0 - Initial release
//     1.1 - "FPS Unlock" & "Aspect Correction" improvements, "Launcher Check",
//           "Skip Logo&Legals" & "FPS Lock" added, "Force Resolution" bugfix.
//
// Copyright (c) 2021 Václav AKA Vaana
//-----------------------------------------------------------------------------

#include <windows.h>

#include "exports.h"
#include "lanPatch.h"
#include "config.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Enable DirectInput proxy
		LoadDinput8();

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

			int width	= Config::options->forceResolutionWidth;
			int height	= Config::options->forceResolutionHeight;
			if (width != 0 && height != 0)
				Patcher::ForceResolution(width, height);

			if (Config::options->forceBorderlessWindow)
				Patcher::ForceBorderless();

			if (Config::options->skipLogos)
				Patcher::SkipLogoAndLegals();

			if (Config::options->skipLauncherCheck)
				Patcher::SkipLauncherCheck();
		}

		Config::Destroy();

		break;
	}     
	case DLL_PROCESS_DETACH: break;
	default: break;
	}
	return TRUE;
}

