// ----------------------------------------------------------------------------
// config.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Options
{
	bool	fps_unlock = true;
	bool	resolution_unlock = true;
	bool	aspect_correction = true;
	double	fps_limit = 0.0;
	float	fov_multiplier = 1.0f;
	bool	borderless_window = true;
	bool	skip_logos_and_legals = false;
};

bool LoadConfig(const TCHAR* path, Options& options);
bool MakeConfig(const TCHAR* path);
