// ----------------------------------------------------------------------------
// fix_fps.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "fix_fps.h"

#include "patching.h"
#include "shared.h"
#include <cassert>

byte sigFramerateDividerConstructor[] =
{
	0x89, 0x5C, 0x24, 0x18,
	0x64, 0x8B, 0x15, 0x2C, 0x00, 0x00, 0x00,
	0xF3, 0x0F, 0x10, 0x05, MASK, MASK, MASK, MASK,
	0xC7, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x8B, 0x02,
	0xC7, 0x06, MASK, MASK, MASK, MASK
};

byte sigFramerateDividerGameplay[] =
{
	0xFE, 0x0D, MSK1, MSK1, MSK1, MSK1,
	0xFF, 0xD2,
	0x8B, 0x0D, MSK1, MSK1, MSK1, MSK1,
	0xC7, 0x46, 0x04, 0x02, 0x00, 0x00, 0x00,
	0x8B, 0x01,
	0x8B, 0x50, 0x40
};

byte sigWaitAndHook[] =
{
	0x0F, 0x57, 0xC0,
	0x0F, 0x2F, 0x44, 0x24, 0x0C,
	0x76, MASK,
	0xD9, 0x44, 0x24, 0x0C,
	0xDC, 0x0D, MASK, MASK, MASK, MASK,
	0xD9, 0x7C, 0x24, 0x0C
};

void RegisterPatch_Framerate()
{
	Patch patch;

	REGISTER_ENGINE_MASK(patch);
	REGISTER_MASK(patch, sigFramerateDividerConstructor, MASK, 21);
	REGISTER_MASK(patch, sigFramerateDividerGameplay, MSK1, 17);
	REGISTER_MASK(patch, sigWaitAndHook, MASK, 8);

	ua_tcscpy_s(patch.name, TEXT("Framerate Unlock"));
	patch.func = ApplyPatch_Framerate;

	RegisterPatch(patch);
}

static I3DEngine** ppEngine;
static LARGE_INTEGER lastTime, timeFrequency;

static float frm = 0.033333f;

bool ApplyPatch_Framerate(Patch* patch)
{
	assert(patch->numSignatures == 4);
	Signature& enginePtr = patch->signatures[0];
	Signature& framerateDividerConstructor = patch->signatures[1];
	Signature& framerateDividerGameplay = patch->signatures[2];
	Signature& waitAndHook = patch->signatures[3];

	// Find the engine pointer
	MemRead(enginePtr.foundPtr, &ppEngine, sizeof(ppEngine));

	// Remove framerate divider
	static unsigned int newFramerateDivider = 1;
	MemWrite(framerateDividerConstructor.foundPtr, &newFramerateDivider, sizeof(newFramerateDivider));
	MemWrite(framerateDividerGameplay.foundPtr, &newFramerateDivider, sizeof(newFramerateDivider));

	// Remove waiting logic and add hook
	jmp jmp;
	MemRead(waitAndHook.foundPtr, &jmp, sizeof(jmp));
	MemWriteHookCall(waitAndHook.foundPtr, &Hook_Frame);
	jmp.opcode = 0xEB;
	jmp.offset -= 5;
	MemWrite((BYTE*)waitAndHook.foundPtr + 5, &jmp, sizeof(jmp));

	// Prepare required variables
	QueryPerformanceCounter(&lastTime);
	QueryPerformanceFrequency(&timeFrequency);


	void* p = (void*)0x00E56E3D;
	DWORD a = (DWORD)&frm;
	MemWrite(p, &a, sizeof(a));

	return true;
}

void Hook_Frame()
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
			engine->framerate = max(fps, 25);
		}
	}

	lastTime = currTime;
}
