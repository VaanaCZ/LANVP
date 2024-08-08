// ----------------------------------------------------------------------------
// opt_skip_logos_and_legals.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "opt_skip_logos_and_legals.h"

#include "patching.h"
#include <cassert>

DWORD sigLogos[] =
{
		0xE8, MASK, MASK, MASK, MASK,
HERE,	0xD9, 0xE8,
		0xA1, MASK, MASK, MASK, MASK,
		0x83, 0xC4, 0x04,
		0x68, MASK, MASK, MASK, MASK,
		0x51,
		0xD9, 0x1C, 0x24,
		0xE8, MASK, MASK, MASK, MASK,
		0xE8, MASK, MASK, MASK, MASK,
		0x8B, 0x44, 0x24, MASK,
		0xE8, MASK, MASK, MASK, MASK,
		0x84, 0xC0
};

DWORD sigLegals[] =
{
HERE,	0x56,
		0x6A, 0x04,
		0x68, MASK, MASK, MASK, MASK,
		0xBE, MASK, MASK, MASK, MASK,
		0xE8, MASK, MASK, MASK, MASK,
		0x68, MASK, MASK, MASK, MASK,
		0xC7, 0x05, MASK, MASK, MASK, MASK, MASK, MASK, MASK, MASK,
		0xE8, MASK, MASK, MASK, MASK,
		0x83, 0xC4, 0x04,
		0x5E
};

static int logosIndex = -1;
static int legalsIndex = -1;

bool sigLegalsFilter(void* ptr)
{
	char* str = nullptr;
	MemRead((BYTE*)ptr + 4, &str, sizeof(str));
	return strcmp(str, "UILegalsScreen") == 0;
}

void RegisterPatch_SkipLogosAndLegals()
{
	Patch patch;

	logosIndex	= patch.AddSignature(SIGARG(sigLogos));
	legalsIndex	= patch.AddSignatureWithFilter(SIGARG(sigLegals), &sigLegalsFilter);

	patch.SetName(L"Skip Logos and Legals option");
	patch.func = &ApplyPatch_SkipLogosAndLegals;

	RegisterPatch(patch);
}

bool ApplyPatch_SkipLogosAndLegals(Patch* patch)
{
	assert(patch->numSignatureIndices == 2);
	void* logos		= patch->GetSignature(logosIndex);
	void* legals	= patch->GetSignature(legalsIndex);

	// Logos
	if (!MemWriteNop(logos,				7))		return false;
	if (!MemWriteNop((BYTE*)logos + 10,	14))	return false;

	// Legals
	BYTE legalsSkip[] =
	{
		0xB8, 0x00, 0x00, 0x00, 0x00,	// mov eax,00000000
		0xC3							// ret
	};

	if (!MemWrite(legals, legalsSkip, sizeof(legalsSkip)))	return false;

	return true;
}
