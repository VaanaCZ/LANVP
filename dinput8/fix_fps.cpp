// ----------------------------------------------------------------------------
// fix_fps.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "fix_fps.h"

#include "patching.h"
#include "shared.h"
#include <cassert>

byte framerateSignature[] =
{
	0x8B, 0xFB,
	0xC6, 0x44, 0x24, 0x48, 0x01,
	0xE8, MASK, MASK, MASK, MASK,
	0x88, 0x44, 0x24, 0x1B,
	0x84, 0xC0
};

void RegisterPatch_Framerate()
{
	Patch patch;

	REGISTER_ENGINE_MASK(patch);
	REGISTER_MASK(patch, framerateSignature, MASK, 7);

	ua_tcscpy_s(patch.name, TEXT("Framerate Unlock"));
	patch.func = ApplyPatch_Framerate;

	RegisterPatch(patch);
}

static I3DEngine** ppEngine;
static LARGE_INTEGER lastTime, timeFrequency;

static float frm = 0.033333f;

bool ApplyPatch_Framerate(Patch* patch)
{
	assert(patch->numSignatures == 2);
	Signature& enginePtr = patch->signatures[0];
	Signature& signature = patch->signatures[1];

	// Find the engine pointer
	MemRead(enginePtr.foundPtr, &ppEngine, sizeof(ppEngine));

	// Patching
	//MemWriteHookCall(signature.foundPtr, &Hook_Framerate);

	byte b = 0x1;

	void* pa = (void*)0x00A8EC0C;
	MemWrite(pa, &b, sizeof(b));

	void* pb = (void*)0x00DF4922;
	MemWrite(pb, &b, sizeof(b));

	void* pc = (void*)0x00A8E9BF;
	MemWriteNop(pc, 6);

	// Prepare required variables
	QueryPerformanceCounter(&lastTime);
	QueryPerformanceFrequency(&timeFrequency);


	void* p = (void*)0x00E56E3D;
	DWORD a = (DWORD)&frm;
	MemWrite(p, &a, sizeof(a));

	return true;
}

char Hook_Framerate(int pointer)
{
	I3DEngine* engine = *ppEngine; 
	assert(engine);

	LARGE_INTEGER currTime;
	QueryPerformanceCounter(&currTime);

	LONGLONG quadDiff = currTime.QuadPart - lastTime.QuadPart;

	if (quadDiff > 0)
	{
		double fps = timeFrequency.QuadPart / (double)quadDiff;


		frm = 1 / fps;

		if (engine)
		{
			engine->framerate = max(fps, 25) * 2.0;
		}
	}

	lastTime = currTime;

	return 1;
}
