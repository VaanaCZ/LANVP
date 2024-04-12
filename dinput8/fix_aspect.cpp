// ----------------------------------------------------------------------------
// fix_aspect.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "fix_aspect.h"

#include "patching.h"
#include "shared.h"
#include <cassert>

DWORD sigBlackBars[] =
{
		0xDF, 0xF1,
		0xDD, 0xD8,
HERE,	0x76, MASK,
		0x89, 0x4C, 0x24, 0x24,
		0xDB, 0x44, 0x24, 0x24,
};

DWORD sigBlackBarsOnResize[] =
{
		0xDF, 0xF1,
		0xDD, 0xD8,
HERE,	0x76, MASK,
		0x8B, 0xD7,
		0x89, 0x54, 0x24, 0x2C,
		0xDB, 0x44, 0x24, 0x2C
};

void RegisterPatch_Aspect()
{
	Patch patch;

	REGISTER_MASK(patch, sigBlackBars);
	REGISTER_MASK(patch, sigBlackBarsOnResize);

	ua_tcscpy_s(patch.name, TEXT("Aspect-ratio fix"));
	patch.func = ApplyPatch_Aspect;

	RegisterPatch(patch);
}

static int uiWidth = 1280.0f;
static int uiHeight = 720.0f;

bool ApplyPatch_Aspect(Patch* patch)
{
	assert(patch->numSignatures == 2);
	void* blackBars			= patch->signatures[0].foundPtr;
	void* blackBarsOnResize	= patch->signatures[1].foundPtr;

	// Remove black bars
	BYTE jmp = 0xEB;
	MemWrite(blackBars,			&jmp, sizeof(jmp));
	MemWrite(blackBarsOnResize,	&jmp, sizeof(jmp));

	// Fix UI scaling


	// Correct FoV

	/*
	
	01259AF8
	
	*/


	return true;
}

int __cdecl Hook_Atoi(const char* string)
{
	return atoi(string);
}
