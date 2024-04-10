#include "fix_aspect.h"

#include "patching.h"
#include "shared.h"
#include <cassert>

#define MASK 0xFF

byte barsSignature[] =
{
	0xDF, 0xF1,
	0xDD, 0xD8,
	0x76, MASK,
	0x89, 0x4C, 0x24, 0x24,
	0xDB, 0x44, 0x24, 0x24,
};

byte barsResizeSignature[] =
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

	Signature signature2;
	REGISTER_MASK(signature2, barsSignature, MASK, 4);
	patch.RegisterSignature(signature2);

	Signature signature3;
	REGISTER_MASK(signature3, barsResizeSignature, MASK, 4);
	patch.RegisterSignature(signature3);

	ua_tcscpy_s(patch.name, TEXT("Aspect-ratio fix"));
	patch.func = ApplyPatch_Aspect;

	RegisterPatch(patch);
}

bool ApplyPatch_Aspect(Patch* patch)
{
	assert(patch->numSignatures == 2);
	Signature& signature2 = patch->signatures[0];
	Signature& signature3 = patch->signatures[1];

	// Removes black bars
	byte jmp = 0xEB;
	WriteProcessMemory(GetCurrentProcess(), signature2.lastOccurence, &jmp, sizeof(jmp), nullptr);

	WriteProcessMemory(GetCurrentProcess(), signature3.lastOccurence, &jmp, sizeof(jmp), nullptr);

	return true;
}
