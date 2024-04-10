// ----------------------------------------------------------------------------
// fix_resolution.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "fix_resolution.h"

#include "patching.h"
#include "shared.h"
#include <cassert>

byte sigAspectCheck[] =
{
	0x8B, 0xC8,
	0x83, 0xC4, 0x04,
	0x8B, 0xC6,
	0xE8, MASK, MASK, MASK, MASK,
	0x84, 0xC0,
	0x0F, 0x84, MASK, MASK, MASK, MASK
};

byte sigOptResCheck[] =
{
	0x57,
	0x8B, 0xF0,
	0xE8, MASK, MASK, MASK, MASK,
	0x84, 0xC0,
	0x74, MASK,
	0x57
};

void RegisterPatch_Resolution()
{
	Patch patch;

	REGISTER_MASK(patch, sigAspectCheck, MASK, 14);
	REGISTER_MASK(patch, sigOptResCheck, MASK, 10);

	ua_tcscpy_s(patch.name, TEXT("Unlock resolutions"));
	patch.func = ApplyPatch_Resolution;

	RegisterPatch(patch);
}

bool ApplyPatch_Resolution(Patch* patch)
{
	assert(patch->numSignatures == 2);
	Signature& aspectCheck = patch->signatures[0];
	Signature& optResCheck = patch->signatures[1];

	// Removes aspect-ratio filter for resolution options
	MemWriteNop(aspectCheck.foundPtr, 6);  // "Resolution:1280x1440x120"

	// Allows any resolution to be selected using -res W H
	MemWriteNop(optResCheck.foundPtr, 2); // "-res 2560 720"

	return true;
}