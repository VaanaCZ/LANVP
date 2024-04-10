#include "fix_resolution.h"

#include "patching.h"
#include <cassert>

#define MASK 0xFF

byte aspectSignature[] =
{
	0x8B, 0xC8,
	0x83, 0xC4, 0x04,
	0x8B, 0xC6,
	0xE8, MASK, MASK, MASK, MASK,
	0x84, 0xC0,
	0x0F, 0x84, MASK, MASK, MASK, MASK
};

byte cccSignature[] =
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

	Signature signature;
	REGISTER_MASK(signature, aspectSignature, MASK, 14);
	patch.RegisterSignature(signature);

	Signature signature4;
	REGISTER_MASK(signature4, cccSignature, MASK, 10);
	patch.RegisterSignature(signature4);

	ua_tcscpy_s(patch.name, TEXT("Resolution fix"));
	patch.func = ApplyPatch_Resolution;

	RegisterPatch(patch);
}

bool ApplyPatch_Resolution(Patch* patch)
{
	assert(patch->numSignatures == 2);
	Signature& signature = patch->signatures[0];
	Signature& signature4 = patch->signatures[1];

	// Allows all resolutions
	byte nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	WriteProcessMemory(GetCurrentProcess(), signature.lastOccurence, nop, sizeof(nop), nullptr);  // "Resolution:1280x1440x120"

	// -res
	byte nop2[] = { 0x90, 0x90 };
	WriteProcessMemory(GetCurrentProcess(), signature4.lastOccurence, nop2, sizeof(nop2), nullptr); // "-res 2560 720"

	return true;
}
