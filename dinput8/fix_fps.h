// ----------------------------------------------------------------------------
// fix_fps.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

struct Patch;

void RegisterPatch_Framerate();
bool ApplyPatch_Framerate(Patch* patch);

char Hook_Framerate(int pointer);