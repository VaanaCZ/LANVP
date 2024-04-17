// ----------------------------------------------------------------------------
// fix_aspect.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "fix_aspect.h"

#include "patching.h"
#include "shared.h"
#include <cassert>
#include <stdlib.h>

DWORD sigBlackBars[] =
{
HERE,	MASK, MASK,
		0x89, 0x4C, 0x24, MASK,
		0xDB, 0x44, 0x24, MASK,
		0xC6, 0x83, 0xA8, 0x0B, 0x00, 0x00, 0x01,
		0x85, 0xC9
};

DWORD sigBlackBarsOnResize[] =
{
HERE,	MASK, MASK,
		0x8B, 0xD7,
		0x89, 0x54, 0x24, MASK,
		0xDB, 0x44, 0x24, MASK,
		0xC6, 0x85, 0xA8, 0x0B, 0x00, 0x00, 0x01,
		0x85, 0xD2
};

void RegisterPatch_Aspect()
{
	Patch patch;

	REGISTER_MASK(patch, sigBlackBars);
	REGISTER_MASK(patch, sigBlackBarsOnResize);

	ua_tcscpy_s(patch.name, 50, TEXT("Aspect-ratio fix"));
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
	if (!MemWrite(blackBars,			&jmp, sizeof(jmp)))		return false;
	if (!MemWrite(blackBarsOnResize,	&jmp, sizeof(jmp)))		return false;

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
