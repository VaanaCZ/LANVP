#pragma once

struct Patch;

void RegisterPatch_Framerate();
void DoPatch_Framerate(Patch* patch);

char DoHook_Framerate(int pointer);
