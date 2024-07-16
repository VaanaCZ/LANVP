// ----------------------------------------------------------------------------
// config.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "config.h"

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include "shared.h"

#define READ_INTEGER(o, k, v, n)	\
if (strcmp(k, #n) == 0)				\
{									\
	o.n = atoi(v);					\
}

#define READ_DOUBLE(o, k, v, n)		\
if (strcmp(k, #n) == 0)				\
{									\
	o.n = atof(v);					\
}

bool LoadConfig(const TCHAR* path, Options& options)
{
	// Load file
	FILE* file = nullptr;

	if (_tfopen_s(&file, path, TEXT("r")) != 0)
	{
		return false;
	}

	char line[128];
	while (fgets(line, sizeof(line) / sizeof(line[0]), file))
	{
		char* c = strchr(line, ';');
		if (c >= line)
			*c = NULL;

		char* tr = strchr(line, '\r');
		if (tr >= line)
			*tr = NULL;

		char* tn = strchr(line, '\n');
		if (tn >= line)
			*tn = NULL;

		if (strlen(line) < 2)
			continue;

		// Read options
		char* e = strchr(line, '=');

		if (e >= line)
		{
			*e = NULL;

			char* key = line;
			char* val = e + 1;

			if (strlen(key) == 0 || strlen(val) == 0)
			{
				continue;
			}

			// Fill options struct
			READ_INTEGER(options, key, val, fps_unlock);
			READ_INTEGER(options, key, val, resolution_unlock);
			READ_INTEGER(options, key, val, aspect_correction);
			READ_DOUBLE	(options, key, val, fps_limit);
			READ_DOUBLE	(options, key, val, fov_multiplier);
			READ_INTEGER(options, key, val, borderless_window);
			READ_INTEGER(options, key, val, skip_logos_and_legals);
		}
	}

	fclose(file);

	return true;
}

const char config[] =
	"; L.A. Noire: V-Patch\n"
	"; Version " VPATCH_VERSION " - (c) 2021-2024 Vaana\n"
	"\n"
	"[fixes]\n"
	"\n"
	"; Removes the default 30 FPS framerate cap and \n"
	"; allows for a varible framerate. Also includes\n"
	"; fixes for problems related to high FPS.\n"
	";\n"
	"; IF YOU CANNOT PROGRESS DUE TO AN FPS BUG, DISABLE THIS OPTION!\n"
	";\n"
	"; 1 = enabled (default)\n"
	"; 0 = disabled\n"
	"fps_unlock=1\n"
	"\n"
	"; Removes resolution filtering, allowing for non-standard\n"
	"; resolutions to be selected in the options menu.\n"
	"; Also allows any resolution to be selected using the\n"
	"; command-line argument \"-res W H\"\n"
	";\n"
	"; 1 = enabled (default)\n"
	"; 0 = disabled\n"
	"resolution_unlock=1\n"
	"\n"
	"; Removes black bars on non-16:9 resolutions, rescales the\n"
	"; GUI to match the screen and corrects the FoV according\n"
	"; to the current aspect ratio.\n"
	";\n"
	"; 1 = enabled (default)\n"
	"; 0 = disabled\n"
	"aspect_correction=1\n"
	"\n"
	"[options]\n"
	"\n"
	"; Allows to set a custom framerate limit.\n"
	"; Useful for a fluctuating framerate.\n"
	"; Only works if fps_unlock is also enabled!\n"
	";\n"
	"; 0 = disabled (default)\n"
	"; example: fps_lock=30\n"
	"fps_limit=0\n"
	"\n"
	"; Allows to set a custom field of view.\n"
	"; Only works if fps_unlock is also enabled!\n"
	";\n"
	"; 1.0 = disabled (default)\n"
	"; example: aspect_correction=1.5\n"
	"fov_multiplier=1.0\n"
	"\n"
	"; Starts the game in a borderless window.\n"
	"; Allows for faster switching between windows.\n"
	"; Only works if the \"Fullscreen\" option is set to\n"
	"; \"NO\" in the settings.ini file.\n"
	";\n"
	"; 1 = enabled (default)\n"
	"; 0 = disabled\n"
	"borderless_window=1\n"
	"\n"
	"; Skips the company logos and legal information which\n"
	"; appears when launching the game.\n"
	";\n"
	"; 1 = enabled\n"
	"; 0 = disabled (default)\n"
	"skip_logos_and_legals=0\n"
	"\n"
	"\n"
	"; The force_resolution option is no longer supported.\n"
	"; Use the command-line argument \"-res W H\" instead.\n"
	"; example: LaNoire.exe -res 1280 720\n"
	"";

bool MakeConfig(const TCHAR* path)
{
	FILE* file = nullptr;

	if (_tfopen_s(&file, path, TEXT("w")) != 0)
	{
		return false;
	}

	fwrite(config, sizeof(config[0]), (sizeof(config) / sizeof(config[0])) - 1, file);

	fclose(file);

	return true;
}
