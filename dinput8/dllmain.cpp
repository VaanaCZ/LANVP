//-----------------------------------------------------------------------------
// dllmain.cpp
// 
// Releases:
//     1.0 - Initial release
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

		if (Config::options.patchEnabled &&
			Patcher::Init())
		{
			// Patches
			if (Config::options.fpsUnlock)
				Patcher::PatchFramerate();

			if (Config::options.aspectUnlock)
				Patcher::PatchViewportAspect();

			if (Config::options.fovRecalculation)
				Patcher::PatchFieldOfView();
			
			// Options
			Patcher::SetFOVMultiplier(Config::options.fovMultiplier);

			int width	= Config::options.forceResolutionWidth;
			int height	= Config::options.forceResolutionHeight;
			int refresh	= Config::options.forceResolutionRefresh;
			if (width != 0 && height != 0 && refresh != 0)
				Patcher::ForceResolution(width, height, refresh);

			if (Config::options.forceBorderlessWindow)
				Patcher::ForceBorderless();
		}

		break;
	}     
	case DLL_PROCESS_DETACH: break;
	default: break;
	}
	return TRUE;
}

