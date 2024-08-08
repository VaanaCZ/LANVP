// ----------------------------------------------------------------------------
// fix_resolution.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "fix_resolution.h"

#include "patching.h"
#include <cassert>

DWORD sigAspectCheck[] =
{
		0x8B, 0xC8,
		0x83, 0xC4, 0x04,
		0x8B, 0xC6,
		0xE8, MASK, MASK, MASK, MASK,
		0x84, 0xC0,
HERE,	0x0F, 0x84, MASK, MASK, MASK, MASK
};

DWORD sigOptResCheck[] =
{
		0x57,
		0x8B, 0xF0,
		0xE8, MASK, MASK, MASK, MASK,
		0x84, 0xC0,
HERE,	0x74, MASK,
		0x57
};

static int aspectCheckIndex = -1;
static int optResCheckIndex = -1;

void RegisterPatch_Resolution()
{
	Patch patch;

	aspectCheckIndex = patch.AddSignature(SIGARG(sigAspectCheck));
	optResCheckIndex = patch.AddSignature(SIGARG(sigOptResCheck));

	patch.SetName(L"Unlock Resolutions");
	patch.func = &ApplyPatch_Resolution;

	RegisterPatch(patch);
}

bool ApplyPatch_Resolution(Patch* patch)
{
	assert(patch->numSignatureIndices == 2);
	void* aspectCheck = patch->GetSignature(aspectCheckIndex);
	void* optResCheck = patch->GetSignature(optResCheckIndex);

	// Removes aspect-ratio filter for resolution options
	if (!MemWriteNop(aspectCheck, 6))	return false; // "Resolution:1280x1440x120"

	// Allows any resolution to be selected using -res W H
	if (!MemWriteNop(optResCheck, 2))	return false; // "-res 2560 720"

	return true;
}
