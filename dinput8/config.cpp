//-----------------------------------------------------------------------------
// config.cpp
// 
// Releases:
//     1.0 - Initial release
//     1.1 - "FPS Unlock" & "Aspect Correction" improvements, "Launcher Check",
//           "Skip Logo&Legals" & "FPS Lock" added, "Force Resolution" bugfix.
//
// Copyright (c) 2021 Václav AKA Vaana
//-----------------------------------------------------------------------------

#include "config.h"

void Config::Init()
{
	//
	// Load and parse ini
	//
	options = new Options;

	options->patchEnabled			= true;
	options->fpsUnlock				= true;
	options->aspectCorrection		= true;
	options->fpsLock				= 0;
	options->fovMultiplier			= 1.0f;
	options->skipLauncherCheck		= false;
	options->skipLogos				= false;
	options->forceBorderlessWindow	= true;
	options->forceResolutionWidth	= 0;
	options->forceResolutionHeight	= 0;

	//
	// Since version 2663, R* has removed 32-bit support, because
	// the new launcher is 64-bit. However, the game itself is
	// still 32-bit. Here we detect if an 32-bit OS	is running,
	// and skip the launcher check if it is, to restore support.
	//

	typedef BOOL(WINAPI* IsWow64Process_t) (HANDLE, PBOOL);
	IsWow64Process_t pIsWow64Process;
	BOOL IsWow64 = FALSE;

	// IsWow64Process must be loaded dynamically, since some older
	// Windows versions don't have this function available.
	pIsWow64Process = (IsWow64Process_t)GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process");

	if (pIsWow64Process == NULL ||
		!pIsWow64Process(GetCurrentProcess(), &IsWow64) ||
		!IsWow64)
	{
		options->skipLauncherCheck = true;
	}

	// todo: Detect if the user has an unsupported resolution

	if (ini_parse(INI_FILE, Handler, options) < 0 || generateNew)
	{
		GenerateConfig();
	}
}



int Config::Handler(void* user, const char* section, const char* name, const char* value)
{
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	#define ERROR(msg)	MessageBoxW(NULL, msg, L"[V-Patch] Error while loading config.", MB_OK);

	Options* pOptions = (Options*)user;

	//
	// Determines whether V-Patch is loaded or not.
	// Setting this value to 0 will disable all subsequent options.
	//
	// 1 = enabled (default)
	// 0 = disabled
	//
	if (MATCH("general", "patch_enabled"))
	{
		int patchEnabled = atoi(value);

		if (patchEnabled < 0 || patchEnabled > 1)
		{
			ERROR(L"patch_enabled was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->patchEnabled = patchEnabled;
	}

	//
	// Removes the FPS cap and corrects the car braking values.
	//
	// If you are unable to progress due to an issue caused by
	// the unlocked framerate, set this option to 0, pass the 
	// current mission and re-enable the option by setting it to 1.
	// 
	// 1 = enabled (default)
	// 0 = disabled
	//
	else if (MATCH("patches", "fps_unlock"))
	{
		int fpsUnlock = atoi(value);

		if (fpsUnlock < 0 || fpsUnlock > 1)
		{
			ERROR(L"fps_unlock was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->fpsUnlock = fpsUnlock;
	}

	//
	// Removes black bars on aspect ratios slimmer than 16:9 
	// (16:10, 4:3, 5:4) and corrects the FOV value and
	// interface size to match the current aspect ratio.
	// 
	// 1 = enabled (default)
	// 0 = disabled
	//
	else if (MATCH("patches", "aspect_correction"))
	{
		int aspectCorrection = atoi(value);

		if (aspectCorrection < 0 || aspectCorrection > 1)
		{
			ERROR(L"aspect_correction was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->aspectCorrection = aspectCorrection;
	}

	//
	// Allows the user to set a custom FPS cap. Useful if
	// your framerate fluctuates rapidly, making the game
	// unplayable. This value should not be lower than 30!
	// fps_unlock must be enabled for this to work!
	//
	// 0 = disabled (default)
	// example: fps_lock=30
	//
	else if (MATCH("options", "fps_lock"))
	{
		int fpsLock = atoi(value);

		if (fpsLock != 0 && fpsLock < 30)
		{
			ERROR(L"fps_lock was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->fpsLock = fpsLock;
	}

	//
	// Allows the user to increase/decrease the fov, in case
	// the game feels too zoomed in/out. It is recommended
	// that this value does not exceed 2.0!
	// fov_correction must be enabled for this to work!
	//
	// default: 1.0
	//
	else if (MATCH("options", "fov_multiplier"))
	{
		float fovMultiplier = atof(value);

		if (fovMultiplier <= 0.0f)
		{
			ERROR(L"fov_multiplier was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->fovMultiplier = fovMultiplier;
	}

	//
	// Skips the intro logos and legal screen which plays
	// every time the game is launched. Useful for modding.
	//
	// 1 = enabled (default)
	// 0 = disabled
	//
	else if (MATCH("options", "skip_launcher_check"))
	{
		int skipLauncherCheck = atoi(value);

		if (skipLauncherCheck < 0 || skipLauncherCheck > 1)
		{
			ERROR(L"skip_launcher_check was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->skipLauncherCheck = skipLauncherCheck;
	}

	//
	// Skips the intro logos and legal screen which plays
	// every time the game is launched. Useful for modding.
	//
	// 1 = enabled (default)
	// 0 = disabled
	//
	else if (MATCH("options", "skip_logos"))
	{
		int skipLogos = atoi(value);

		if (skipLogos < 0 || skipLogos > 1)
		{
			ERROR(L"skip_logos was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->skipLogos = skipLogos;
	}

	//
	// Starts the game in borderless window mode. Also has
	// benefit of removing v-sync. Recommended for faster
	// switching between applications.
	//
	// 1 = enabled (default)
	// 0 = disabled
	//
	else if (MATCH("options", "force_borderless_window"))
	{
		int forceBorderlessWindow = atoi(value);

		if (forceBorderlessWindow < 0 || forceBorderlessWindow > 1)
		{
			ERROR(L"force_borderless_window was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->forceBorderlessWindow = forceBorderlessWindow;
	}

	//
	// Allows you to set a custom resolution not supported
	// by the game, as it is in case of ultrawide resolutions.
	//
	// By default, this is set to your current resolution.
	// Leave this option blank to disable it.
	// 
	// example: force_resolution=1920x1080
	//
	else if (MATCH("options", "force_resolution"))
	{
		int width = atoi(value);
		const char* fx = (char*)((int)strchr(value, 'x') + 1);
		if (fx == (char*)(NULL + 1))
			return 0;

		int height = atoi(fx);

		if (width < 0 || height < 0)
			return 0;

		pOptions->forceResolutionWidth      = width;
		pOptions->forceResolutionHeight     = height;
	}

	else
	{
		if (!generateNew)
		{
			ERROR(L"Config is invalid. A new config will be generated.");
			generateNew = true;
		}		

		return 0;
	}

	return 1;
}

bool Config::GenerateConfig()
{
	#define ERROR(msg)	MessageBoxW(NULL, msg, L"[V-Patch] Error while generating config.", MB_OK);

	int width, height;

	//
	// Gather the necessary info to create a new config.
	//
	DEVMODEW devMode = { };
	devMode.dmSize = sizeof(DEVMODE);
	devMode.dmDriverExtra = 0;

	if (EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &devMode))
	{
		width	= devMode.dmPelsWidth;
		height	= devMode.dmPelsHeight;
	}
	else
	{
		width = height = 0;
	}

	//
	// Fill out template and write to disk.
	//
	size_t configSize = sizeof(configTemplate) + 32;
	char* config = new char[configSize];

	size_t configBytesWritten = sprintf_s(config, configSize, configTemplate, options->skipLauncherCheck, width, height);
	if (configBytesWritten == -1 || configBytesWritten == 0)
	{
		ERROR(L"Failed to create a new config. sprintf_s didn't write any characters.");
		return false;
	}

	HANDLE file = CreateFileW(TEXT(INI_FILE), GENERIC_WRITE, FILE_SHARE_READ, NULL,
							  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);                

	if (file == INVALID_HANDLE_VALUE)
	{
		ERROR(L"Failed to create a new config. File handle invalid.");
		return false;
	}

	DWORD fileBytesWritten;
	if (!WriteFile(file, config, configBytesWritten, &fileBytesWritten, nullptr) ||
		fileBytesWritten == 0)
	{
		ERROR(L"Failed to create a new config. No bytes written.");
		return false;
	}

	delete[] config;
	CloseHandle(file);

	return true;
}
