//-----------------------------------------------------------------------------
// config.h
// 
// Releases:
//     1.0 - Initial release
//     1.1 - "FPS Unlock" & "Aspect Correction" improvements, "Launcher Check",
//           "Skip Logo&Legals" & "FPS Lock" added, "Force Resolution" bugfix.
//
// Copyright (c) 2021 Václav AKA Vaana
//-----------------------------------------------------------------------------

#pragma once

#define INI_FILE "lanvp.ini"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "inih\ini.h"
#include "lanConstants.h"

class Config
{
public:

	static void Init();
	static int	Handler(void*, const char*, const char*, const char*);
	static void Destroy() { delete options; }

	static bool GenerateConfig();

	struct Options
	{
		bool	patchEnabled;
		bool	fpsUnlock;
		bool	aspectCorrection;
		int		fpsLock;
		float	fovMultiplier;
		bool	skipLauncherCheck;
		bool	skipLogos;
		bool	forceBorderlessWindow;
		int		forceResolutionWidth;
		int		forceResolutionHeight;
	};

	inline static bool		generateNew = false;
	inline static Options*	options;

};

const char configTemplate[] =	"; L.A. Noire: V-Patch\r\n"
								"; Version " VPATCH_VERSION " - (c) 2021\r\n"
								"\r\n"
								"[general]\r\n"
								"\r\n"
								";\r\n"
								"; Determines whether V-Patch is loaded or not.\r\n"
								"; Setting this value to 0 will disable all subsequent options.\r\n"
								";\r\n"
								"; 1 = enabled (default)\r\n"
								"; 0 = disabled\r\n"
								"patch_enabled=1\r\n"
								"\r\n"
								"\r\n"
								"[patches]\r\n"
								"\r\n"
								";\r\n"
								"; Removes the FPS cap and corrects the car braking values.\r\n"
								";\r\n"
								"; If you are unable to progress due to an issue caused by\r\n"
								"; the unlocked framerate, set this option to 0, pass the \r\n"
								"; current mission and re-enable the option by setting it to 1.\r\n"
								"; \r\n"
								"; 1 = enabled (default)\r\n"
								"; 0 = disabled\r\n"
								"fps_unlock=1\r\n"
								"\r\n"
								"\r\n"
								";\r\n"
								"; Removes black bars on aspect ratios slimmer than 16:9 \r\n"
								"; (16:10, 4:3, 5:4) and corrects the FOV value and \r\n"
								"; interface size to match the current aspect ratio.\r\n"
								"; \r\n"
								"; 1 = enabled (default)\r\n"
								"; 0 = disabled\r\n"
								"aspect_correction=1\r\n"
								"\r\n"
								"\r\n"
								"[options]\r\n"
								"\r\n"
								";\r\n"
								"; Allows the user to set a custom FPS cap. Useful if\r\n"
								"; your framerate fluctuates rapidly, making the game\r\n"
								"; unplayable. This value should not be lower than 30!\r\n"
								"; fps_unlock must be enabled for this to work!\r\n"
								";\r\n"
								"; 0 = disabled (default)\r\n"
								"; example: fps_lock=30\r\n"
								"fps_lock=0\r\n"
								"\r\n"
								"\r\n"
								";\r\n"
								"; Lets the user increase/decrease the fov, in case\r\n"
								"; the game feels too zoomed in/out. It is recommended\r\n"
								"; that this value does not exceed 2.0!\r\n"
								"; aspect_correction must be enabled for this to work!\r\n"
								";\r\n"
								"; default: 1.0\r\n"
								"fov_multiplier=1.0\r\n"
								"\r\n"
								"\r\n"
								";\r\n"
								"; Makes the game run without needing to be started through\r\n"
								"; the LAN Launcher. Does not work for R* Launcher builds.\r\n"
								";\r\n"
								"; 1 = enabled\r\n"
								"; 0 = disabled\r\n"
								"skip_launcher_check=%d\r\n"
								"\r\n"
								"\r\n"
								";\r\n"
								"; Skips the intro logos and legals screen which plays\r\n"
								"; every time the game is launched.\r\n"
								";\r\n"
								"; 1 = enabled\r\n"
								"; 0 = disabled (default)\r\n"
								"skip_logos=0\r\n"
								"\r\n"
								"\r\n"
								";\r\n"
								"; Starts the game in borderless window mode. Recommended\r\n"
								"; for faster switching between applications.\r\n"
								";\r\n"
								"; 1 = enabled (default)\r\n"
								"; 0 = disabled\r\n"
								"force_borderless_window=1\r\n"
								"\r\n"
								"\r\n"
								";\r\n"
								"; Allows you to set a custom resolution not supported\r\n"
								"; by the game, as it is in case of ultrawide resolutions.\r\n"
								";\r\n"
								"; By default, this is set to your current resolution.\r\n"
								"; Leave this option blank to disable it.\r\n"
								"; \r\n"
								"; example: force_resolution=1920x1080\r\n"
								"force_resolution=%dx%d\r\n"
								"\r\n";

