//-----------------------------------------------------------------------------
// config.cpp
// 
// Releases:
//     1.0 - Initial release
//
// Copyright (c) 2021 Václav AKA Vaana
//-----------------------------------------------------------------------------

#include "config.h"

void Config::Init()
{
	//
	// Load and parse ini
	//
	options.patchEnabled            = true;
	options.fpsUnlock               = true;
	options.aspectUnlock            = true;
	options.fovRecalculation        = true;
	options.fovMultiplier           = 1.0f;
	options.forceBorderlessWindow   = true;
	options.forceResolutionWidth    = 0;
	options.forceResolutionHeight   = 0;
	options.forceResolutionRefresh  = 0;

	if (ini_parse(INI_FILE, Handler, &options) < 0 || generateNew)
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
	// (16:10, 4:3, 5:4) and allows playing the game in any
	// aspect ratio.
	// 
	// 1 = enabled (default)
	// 0 = disabled
	//
	else if (MATCH("patches", "aspect_unlock"))
	{
		int aspectUnlock = atoi(value);

		if (aspectUnlock < 0 || aspectUnlock > 1)
		{
			ERROR(L"aspect_unlock was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->aspectUnlock = aspectUnlock;
	}

	//
	// Corrects the FOV value to match the current aspect
	// ratio. Primarily useful for ultrawide resolutions.
	//
	// 1 = enabled (default)
	// 0 = disabled
	//
	else if (MATCH("patches", "fov_recalculation"))
	{
		int fovRecalculation = atoi(value);

		if (fovRecalculation < 0 || fovRecalculation > 1)
		{
			ERROR(L"fov_recalculation was set to an invalid value. Using defaults...");
			return 0;
		}

		pOptions->fovRecalculation = fovRecalculation;
	}

	//
	// Allows the user to increase/decrease the fov, in case
	// the game feels too zoomed in/out. It is recommended
	// that this value does not exceed 2.0!
	// fov_recalculation must be enabled for this to work!!!
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
	// example: force_resolution=1920x1080x60
	//
	else if (MATCH("options", "force_resolution"))
	{
		int width = atoi(value);
		const char* fx = (char*)((int)strchr(value, 'x') + 1);
		if (fx == (char*)(NULL + 1))
			return 0;

		int height = atoi(fx);
		const char* sx = (char*)((int)strchr(fx, 'x') + 1);
		if (sx == (char*)(NULL + 1))
			return 0;

		int refresh = atoi(sx);

		if (width < 0 || height < 0 || refresh < 0)
			return 0;

		pOptions->forceResolutionWidth      = width;
		pOptions->forceResolutionHeight     = height;
		pOptions->forceResolutionRefresh    = refresh;
	}

	else
	{
		ERROR(L"Config is invalid. A new config will be generated.");
		generateNew = true;

		return 0;
	}

	return 1;
}

bool Config::GenerateConfig()
{
	#define ERROR(msg)	MessageBoxW(NULL, msg, L"[V-Patch] Error while generating config.", MB_OK);

	int width, height, refresh;

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
		refresh = devMode.dmDisplayFrequency;
	}
	else
	{
		width = height = refresh = 0;
	}

	//
	// Fill out template and write to disk.
	//
	size_t configSize = sizeof(configTemplate) + 32;
	char* config = new char[configSize];

	size_t configBytesWritten = sprintf_s(config, configSize, configTemplate, width, height, refresh);
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
