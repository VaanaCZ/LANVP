// ----------------------------------------------------------------------------
// config.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

#include <windows.h>

struct Options
{
	bool	fps_unlock = true;
	bool	resolution_unlock = true;
	bool	aspect_correction = true;
	double	fps_limit = 0.0;
	bool	borderless_window = true;
	bool	skip_logos_and_legals = false;
};

bool LoadConfig(const TCHAR* path, Options& options);
bool MakeConfig(const TCHAR* path);
