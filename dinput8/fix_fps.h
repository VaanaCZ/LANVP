// ----------------------------------------------------------------------------
// fix_fps.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

struct Patch;

void RegisterPatch_Framerate();
bool ApplyPatch_Framerate(Patch* patch);

extern double minFrameTime;

void __stdcall Hook_Frame();

struct InspectionSystem;
void __stdcall Hook_Pencil(InspectionSystem* inst);
