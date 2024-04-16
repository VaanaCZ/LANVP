// ----------------------------------------------------------------------------
// opt_skip_logos_and_legals.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "opt_skip_logos_and_legals.h"

#include "patching.h"
#include "shared.h"
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
HERE,	0x6A, 0xFF,
		0x68, MASK, MASK, MASK, MASK,
		0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
		0x50,
		0x56,
		0x57,
		0xA1, MASK, MASK, MASK, MASK
};

void RegisterPatch_SkipLogosAndLegals()
{
	Patch patch;

	REGISTER_MASK(patch, sigLogos);
	//REGISTER_MASK(patch, sigLegals);

	ua_tcscpy_s(patch.name, 50, TEXT("Skip logos and legals option"));
	patch.func = ApplyPatch_SkipLogosAndLegals;

	RegisterPatch(patch);
}

bool ApplyPatch_SkipLogosAndLegals(Patch* patch)
{
	//assert(patch->numSignatures == 2);
	void* logos		= patch->signatures[0].foundPtr;
	//void* legals	= patch->signatures[1].foundPtr;

	// Logos
	MemWriteNop(logos,				7);
	MemWriteNop((BYTE*)logos + 10,	14);



	// Legals
	//BYTE legalsSkip[] =
	//{
	//	0xB8, 0x00, 0x00, 0x00, 0x00,	// mov eax,00000000
	//	0xC3							// ret
	//};
	//
	//MemWrite(legals, legalsSkip, sizeof(legalsSkip));

	return true;
}
