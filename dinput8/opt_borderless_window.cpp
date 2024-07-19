// ----------------------------------------------------------------------------
// opt_borderless_window.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "opt_borderless_window.h"

#include "patching.h"
#include "shared.h"
#include <cassert>

DWORD sigAdjustWindowRect[] =
{
HERE,	0xC7, 0x44, 0x24, 0x2C, MASK, MASK, MASK, MASK,
		0x89, MASK, 0x24, 0x30,
		0x89, MASK, 0x24, 0x34,
		0x89, MASK, 0x24, 0x38,
		0x89, MASK, 0x24, 0x3C,
		0xFF, 0x15, MASK, MASK, MASK, MASK,
		0x8B, MASK, 0x24, 0x2C,
		0x2B, MASK, 0x24, 0x24,
		0x8B, MASK, 0x24, 0x30,
		0x2B, MASK, 0x24, 0x28,
		0x8B, 0x3E,
		0x8B, 0x6E, 0x04
};

static int adjustWindowRectIndex = -1;

void RegisterPatch_BorderlessWindow()
{
	Patch patch;

	adjustWindowRectIndex = patch.AddSignature(SIGARG(sigAdjustWindowRect));

	ua_tcscpy_s(patch.name, 50, TEXT("Borderless window option"));
	patch.func = ApplyPatch_BorderlessWindow;

	RegisterPatch(patch);
}

bool ApplyPatch_BorderlessWindow(Patch* patch)
{
	assert(patch->numSignatureIndices == 1);
	void* adjustWindowRect = patch->GetSignature(adjustWindowRectIndex);

	// Change style to borderless
	DWORD style = WS_POPUP;
	if (!MemWrite((BYTE*)adjustWindowRect + 4, &style, sizeof(style)))				return false;

	// Remove AdjustWindowRect call
	BYTE removeCall[] =
	{
		0x58,	// pop eax
		0x58,	// pop eax
		0x58,	// pop eax
		0x90,	// nop
		0x90,	// nop
		0x90	// nop
	};

	if (!MemWrite((BYTE*)adjustWindowRect + 24, removeCall, sizeof(removeCall)))	return false;

	return true;
}
