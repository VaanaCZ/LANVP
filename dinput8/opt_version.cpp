// ----------------------------------------------------------------------------
// opt_version.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "opt_version.h"

#include "patching.h"
#include "shared.h"
#include <cassert>
#include <stdio.h>

DWORD sigVersion[] =
{
		0x51,
		0x89, 0x74, 0x24, 0x38,
		0x89, 0x74, 0x24, 0x2C,
HERE,	0xE8, MASK, MASK, MASK, MASK,
		0x8B, 0x44, 0x24, 0x28,
		0x8B, 0x54, 0x24, 0x1C,
		0x50
};

static int versionIndex = -1;

void RegisterPatch_Version()
{
	Patch patch;

	versionIndex = patch.AddSignature(SIGARG(sigVersion));

	patch.SetName(L"Show the version number in the main menu");
	patch.func = &ApplyPatch_Version;

	RegisterPatch(patch);
}

bool ApplyPatch_Version(Patch* patch)
{
	assert(patch->numSignatureIndices == 1);
	void* version = patch->GetSignature(versionIndex);

	if (!MemWriteHookCall(version, &Hook_VerQueryValueA))	return false;

	return true;
}

BOOL __stdcall Hook_VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen)
{
	BOOL result = VerQueryValueA(pBlock, lpSubBlock, lplpBuffer, puLen);
	char* buffer = (char*)*lplpBuffer;

	char* p = strchr(buffer, ',');
	*p = NULL;
	size_t length = strlen(buffer);

	const size_t bufferLength = 64;
	char newBuffer[bufferLength];
	sprintf_s(newBuffer, bufferLength, "%s; V-Patch v" VPATCH_VERSION_MENU ",%s", buffer, p + 1);

	*lplpBuffer = newBuffer;
	*puLen = strlen(newBuffer);

	return result;
}
