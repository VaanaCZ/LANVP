// ----------------------------------------------------------------------------
// fix_aspect.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "fix_aspect.h"

#include "patching.h"
#include "shared.h"
#include <cassert>

byte sigBlackBars[] =
{
	0xDF, 0xF1,
	0xDD, 0xD8,
	0x76, MASK,
	0x89, 0x4C, 0x24, 0x24,
	0xDB, 0x44, 0x24, 0x24,
};

byte sigBlackBarsOnResize[] =
{
	0xDF, 0xF1,
	0xDD, 0xD8,
	0x76, MASK,
	0x8B, 0xD7,
	0x89, 0x54, 0x24, 0x2C,
	0xDB, 0x44, 0x24, 0x2C
};

void RegisterPatch_Aspect()
{
	Patch patch;

	REGISTER_MASK(patch, sigBlackBars, MASK, 4);
	REGISTER_MASK(patch, sigBlackBarsOnResize, MASK, 4);

	ua_tcscpy_s(patch.name, TEXT("Aspect-ratio fix"));
	patch.func = ApplyPatch_Aspect;

	RegisterPatch(patch);
}

static int uiWidth = 1280.0f;
static int uiHeight = 720.0f;

bool ApplyPatch_Aspect(Patch* patch)
{
	assert(patch->numSignatures == 2);
	Signature& blackBars			= patch->signatures[0];
	Signature& blackBarsOnResize	= patch->signatures[1];

	// Remove black bars
	byte jmp = 0xEB;
	MemWrite(blackBars.foundPtr,			&jmp, sizeof(jmp));
	MemWrite(blackBarsOnResize.foundPtr,	&jmp, sizeof(jmp));

	// Fix UI scaling


	// Correct FoV


	return true;
}

int __cdecl Hook_Atoi(const char* string)
{
	return atoi(string);
}
