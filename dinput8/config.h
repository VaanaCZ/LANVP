#pragma once

#include <windows.h>

struct Options
{
	bool patch_enabled = true;
};

bool LoadConfig(const TCHAR* path, Options& options);
bool MakeConfig(const TCHAR* path);
