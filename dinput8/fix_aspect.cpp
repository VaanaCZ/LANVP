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

DWORD sigUiLayerSize[] =
{
HERE,	0xF2, 0x0F, 0x5E, 0x05, MASK, MASK, MASK, MASK,
		0xF3, 0x0F, 0x5A, 0xC9,
		0xF2, 0x0F, 0x59, 0xC1,
		0xF3, 0x0F, 0x2A, 0x4C, 0x24, 0x04,
		0x66, 0x0F, 0x5A, 0xC0,
		0xF3, 0x0F, 0x11, 0x06,
		0xF3, 0x0F, 0x10, 0x44, 0x24, 0x14,
		0x0F, 0x5A, 0xC0,
		0xF2, 0x0F, 0x5E, 0x05, MASK, MASK, MASK, MASK,
};

DWORD sigUiLayerSize2[] =
{
HERE,	0xF2, 0x0F, 0x5E, 0x05, MASK, MASK, MASK, MASK,
		0xF3, 0x0F, 0x2A, 0x4C, 0x24, 0x10,
		0xF3, 0x0F, 0x2A, 0x54, 0x24, 0x14,
		0xF3, 0x0F, 0x5A, 0xC9,
		0xF2, 0x0F, 0x59, 0xC1,
		0xF3, 0x0F, 0x5A, 0x4C, 0x24, 0x0C,
		0xF2, 0x0F, 0x5E, 0x0D, MASK, MASK, MASK, MASK
};

DWORD sigAltUiLayerSize[] =
{
HERE,	0xDC, 0x35, MASK, MASK, MASK, MASK,
		0x8B, 0xC6,
		0xDA, 0x0C, 0x24,
		0xD9, 0x1E,
		0xD9, 0x44, 0x24, 0x14,
		0xDC, 0x35, MASK, MASK, MASK, MASK
};

DWORD sigUiSubtitleLayer[] =
{
HERE,	0xDC, 0x35, MASK, MASK, MASK, MASK,
		0xF3, 0x0F, 0x10, 0x44, 0x24, 0x10,
		0xF3, 0x0F, 0x11, 0x46, 0x58,
		0xD9, 0x5C, 0x24, 0x0C,
		0xD8, 0x74, 0x24, 0x0C,
		0xDC, 0x3D, MASK, MASK, MASK, MASK,
		0xD9, 0x5C, 0x24, 0x14,
		0xD9, 0x44, 0x24, 0x14,
		0xD9, 0x5E, 0x5C
};

void RegisterPatch_Aspect()
{
	Patch patch;

	REGISTER_MASK(patch, sigBlackBars);
	REGISTER_MASK(patch, sigBlackBarsOnResize);
	REGISTER_MASK_ALTERNATE(patch, sigUiLayerSize, sigAltUiLayerSize);
	REGISTER_MASK_ALTERNATE(patch, sigUiLayerSize2, sigAltUiLayerSize);
	REGISTER_MASK(patch, sigUiSubtitleLayer);

	ua_tcscpy_s(patch.name, 50, TEXT("Aspect-ratio fix"));
	patch.func = ApplyPatch_Aspect;

	RegisterPatch(patch);
}

static double uiWidth = 1280.0f;
static double uiHeight = 720.0f;

bool ApplyPatch_Aspect(Patch* patch)
{
	assert(patch->numSignatures == 5);
	void* blackBars			= patch->signatures[0].foundPtr;
	void* blackBarsOnResize	= patch->signatures[1].foundPtr;
	void* uiLayerSize		= patch->signatures[2].foundPtr;
	void* uiLayerSize2		= patch->signatures[3].foundPtr;
	bool isAlternate		= patch->signatures[2].isAlternate;
	void* uiSubtitleLayer	= patch->signatures[4].foundPtr;

	// Remove black bars
	BYTE jmp = 0xEB;
	if (!MemWrite(blackBars,			&jmp, sizeof(jmp)))		return false;
	if (!MemWrite(blackBarsOnResize,	&jmp, sizeof(jmp)))		return false;

	// Fix scaling of UI layers
	static void* pAtoi = &Hook_Atoi;
	MemWriteHookCallPtr((void*)0x009CFD3D, &pAtoi);

	static double* pUiWidth = &uiWidth;
	static double* pUiHeight = &uiHeight;

	if (!isAlternate)
	{
		void* w1 = (BYTE*)uiLayerSize + 4;
		void* h1 = (BYTE*)uiLayerSize + 43;
		if (!MemWrite(w1, &pUiWidth, sizeof(pUiWidth)))		return false;
		if (!MemWrite(h1, &pUiHeight, sizeof(pUiHeight)))	return false;

		void* w2 = (BYTE*)uiLayerSize2 + 4;
		void* h2 = (BYTE*)uiLayerSize2 + 38;
		if (!MemWrite(w2, &pUiWidth, sizeof(pUiWidth)))		return false;
		if (!MemWrite(h2, &pUiHeight, sizeof(pUiHeight)))	return false;
	}
	else
	{
		void* w1 = (BYTE*)uiLayerSize + 2;
		void* h1 = (BYTE*)uiLayerSize + 19;
		if (!MemWrite(w1, &pUiWidth, sizeof(pUiWidth)))		return false;
		if (!MemWrite(h1, &pUiHeight, sizeof(pUiHeight)))	return false;
	}

	void* w = (BYTE*)uiSubtitleLayer + 2;
	if (!MemWrite(w, &pUiWidth, sizeof(pUiWidth)))			return false;



	/*
	MemWrite((void*)0x009CFD60, &pUiWidth, sizeof(pUiWidth));
	//MemWrite((void*)0x00C64146, &pUiWidth, sizeof(pUiWidth));
	MemWrite((void*)0x00493326, &pUiWidth, sizeof(pUiWidth));
	MemWrite((void*)0x00491FD3, &pUiWidth, sizeof(pUiWidth));
	//MemWrite((void*)0x0048F2C3, &pUiWidth, sizeof(pUiWidth));
	MemWrite((void*)0x0044DE97, &pUiWidth, sizeof(pUiWidth));
	//MemWrite((void*)0x0040F9EB, &pUiWidth, sizeof(pUiWidth));

	//MemWrite((void*)0x00C6416D, &pUiHeight, sizeof(pUiHeight));
	MemWrite((void*)0x00493352, &pUiHeight, sizeof(pUiHeight));
	MemWrite((void*)0x00491FF5, &pUiHeight, sizeof(pUiHeight));
	//MemWrite((void*)0x0048F2E5, &pUiHeight, sizeof(pUiHeight));
	*/
	

	// Correct FoV

	/*
	
	01259AF8
	
	*/


	return true;
}

int __cdecl Hook_Atoi(const char* string)
{
	const char* p = strchr(string, 'x');
	
	int width = atoi(string);
	int height = atoi(p + 1);

	double aspect = width / (double)height;
	double multiplier = aspect / 1.77777778;

	if (multiplier > 1.0)
	{
		uiWidth		= 1280.0 * multiplier;
		uiHeight	= 720.0;
	}
	else
	{
		uiWidth		= 1280.0;
		uiHeight	= 720.0 / multiplier;
	}

	return atoi(string);
}
