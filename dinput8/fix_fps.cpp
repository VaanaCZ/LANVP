#include "fix_fps.h"

#include <cassert>
#include "patching.h"
#include "shared.h"

#define MASK 0xFF

byte framerateSignature[] =
{
	0x8B, 0xFB,
	0xC6, 0x44, 0x24, 0x48, 0x01,
	0xE8, MASK, MASK, MASK, MASK,
	0x88, 0x44, 0x24, 0x1B,
	0x84, 0xC0
};

byte rendererDestructorSignature[] =
{
	0x8B, 0x96, 0x9C, 0x00, 0x00, 0x00,
	0x83, 0xE1, 0xF8,
	0x8B, 0x01,
	0x8B, 0x40, 0x0C,
	0x52,
	0xFF, 0xD0,
	0xC7, 0x05, MASK, MASK, MASK, MASK, 0x00, 0x00, 0x00, 0x00,
	0x8B, 0x4C, 0x24, 0x10,
	0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00
};

void RegisterPatch_Framerate()
{
	Patch patch;
	
	Signature signature;
	REGISTER_MASK(signature, framerateSignature, MASK, 8);
	patch.RegisterSignature(signature);

	Signature signature2;
	REGISTER_MASK(signature2, rendererDestructorSignature, MASK, 19);
	patch.RegisterSignature(signature2);

	ua_tcscpy_s(patch.name, TEXT("Framerate Unlock"));
	patch.func = ApplyPatch_Framerate;

	RegisterPatch(patch);
}

static I3DEngine** ppEngine;
static LARGE_INTEGER lastTime, timeFrequency;

bool ApplyPatch_Framerate(Patch* patch)
{
	assert(patch->numSignatures == 2);
	Signature& signature = patch->signatures[0];
	Signature& signature2 = patch->signatures[1];

	// Patching
	DWORD hookAddress = (DWORD)&Hook_Framerate;
	hookAddress -= (DWORD)signature.lastOccurence;
	hookAddress -= 4;

	WriteProcessMemory(GetCurrentProcess(), signature.lastOccurence, &hookAddress, sizeof(hookAddress), 0);

	ReadProcessMemory(GetCurrentProcess(), signature2.lastOccurence, &ppEngine, sizeof(ppEngine), 0);

	// Prepare vars
	QueryPerformanceCounter(&lastTime);
	QueryPerformanceFrequency(&timeFrequency);

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

		if (engine)
		{
			engine->framerate = max(fps, 25) * 2.0;
		}
	}

	lastTime = currTime;

	return 1;
}
