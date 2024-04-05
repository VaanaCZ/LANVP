#pragma once

struct Patch;

void RegisterPatch_Framerate();
bool ApplyPatch_Framerate(Patch* patch);

char Hook_Framerate(int pointer);
