// ----------------------------------------------------------------------------
// fix_aspect.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

struct Patch;

void RegisterPatch_Aspect();
bool ApplyPatch_Aspect(Patch* patch);

int __cdecl Hook_Atoi(const char* string);

class CameraManager;
void __stdcall Hook_Fov(CameraManager* camera);
