#include "config.h"
#include <stdio.h>
#include <tchar.h>

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
			if (strcmp(key, "patch_enabled") == 0)
			{
				options.patch_enabled = atoi(val);
			}
		}
	}
	
	fclose(file);

	return true;
}

const char config[] =
	"; L.A. Noire: V-Patch\n"
	"; Version 2.0\n"
	"\n"
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
