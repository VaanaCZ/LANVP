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

#include "patching.h"
#include "fix_fps.h"
#include "fix_resolution.h"
#include "fix_aspect.h"

/*BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Enable DirectInput proxy
		LoadDinput8();*/


void f(Patch* patch)
{
	size_t a = 0;
}


/*

todo: introspection?
0x00D1AB60

*/



extern "C" __declspec(dllexport) BOOL InitializeASI()
{
	/*
	byte m[]  = { 0xC1, 0x04, 0x8B, 0x15, 0x18, 0xAE, 0x51, 0x01, 0x50, 0x52, 0xE8, 0x21, 0x06, 0xFA, 0xFF, 0x8B };

	Patch p;
	wcscpy_s(p.name, TEXT("Superduper patch"));
	p.signatures[0].mask = m;
	p.signatures[0].maskLength = sizeof(m);
	p.signatures[0].maskingByte = 0xFF;
	p.func = f;
	p.numSignatures++;


	RegisterPatch(p);


	*/

	RegisterPatch_Framerate();
	RegisterPatch_Resolution();
	RegisterPatch_Aspect();

	DoPatches();





	Config::Init();

	if (Config::options->patchEnabled &&
		Patcher::Init())
	{
		// Patches
		//if (Config::options->fpsUnlock)
		//	Patcher::PatchFramerate();

		if (Config::options->aspectCorrection)
			Patcher::PatchAspect();

		// Options
		Patcher::SetFOVMultiplier(Config::options->fovMultiplier);
		Patcher::SetFPSLock(Config::options->fpsLock);

		//int width = Config::options->forceResolutionWidth;
		//int height = Config::options->forceResolutionHeight;
		//int width = 1280;
		//int height = 1024;
		/*int width = 2560;
		int height = 720;
		if (width != 0 && height != 0)
			Patcher::ForceResolution(width, height);*/

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

