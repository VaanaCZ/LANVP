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

DWORD sigUiSizeHook[] =
{
HERE,	0xFF, 0x15, MASK, MASK, MASK, MASK,
		0x8B, 0xC8,
		0x83, 0xC4, 0x04,
		0x89, 0x4C, 0x24, 0x20,
		0xDB, 0x44, 0x24, 0x20,
		0x89, 0x44, 0x24, 0x14,
		0x85, 0xC9,
		0x7D, 0x06,
		0xD8, 0x05, MASK, MASK, MASK, MASK,
		0xDC, 0x35, MASK, MASK, MASK, MASK
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

DWORD sigUiLegalsScreen[] =
{
HERE,	0xDC, 0x35, MASK, MASK, MASK, MASK,
		0x8B, 0x0D, MASK, MASK, MASK, MASK,
		0xD9, 0x5C, 0x24, 0x04,
		0xD9, 0x44, 0x24, 0x04,
		0xD9, 0x99, 0xCC, 0x00, 0x00, 0x00,
		0x83, 0xC4, 0x08
};

DWORD sigFov[] =
{
		0x0F, 0x57, 0xC0,
		0x0F, 0x29, 0x47, 0x20,
		0x89, 0xBB, 0xE0, 0x00, 0x00, 0x00,
HERE,	0x8B, 0x17,
		0x8B, 0x82, 0xA0, 0x00, 0x00, 0x00,
		0x8B, 0xCF,
		0xFF, 0xD0
};

void RegisterPatch_Aspect()
{
	Patch patch;

	REGISTER_ENGINE_MASK(patch);
	REGISTER_MASK(patch, sigBlackBars);
	REGISTER_MASK(patch, sigBlackBarsOnResize);
	REGISTER_MASK(patch, sigUiSizeHook);
	REGISTER_MASK_ALTERNATE(patch, sigUiLayerSize, sigAltUiLayerSize);
	REGISTER_MASK_ALTERNATE(patch, sigUiLayerSize2, sigAltUiLayerSize);
	REGISTER_MASK(patch, sigUiSubtitleLayer);
	REGISTER_MASK(patch, sigUiLegalsScreen);
	REGISTER_MASK(patch, sigFov);

	ua_tcscpy_s(patch.name, 50, TEXT("Aspect-ratio fix"));
	patch.func = ApplyPatch_Aspect;

	RegisterPatch(patch);
}

static I3DEngine** ppEngine;					// Pointer to engine object

static double uiWidth = 1280.0f;
static double uiHeight = 720.0f;

bool ApplyPatch_Aspect(Patch* patch)
{
	assert(patch->numSignatures == 9);
	void* enginePtr			= patch->signatures[0].foundPtr;
	void* blackBars			= patch->signatures[1].foundPtr;
	void* blackBarsOnResize	= patch->signatures[2].foundPtr;
	void* uiSizeHook		= patch->signatures[3].foundPtr;
	void* uiLayerSize		= patch->signatures[4].foundPtr;
	void* uiLayerSize2		= patch->signatures[5].foundPtr;
	bool isAlternate		= patch->signatures[4].isAlternate;
	void* uiSubtitleLayer	= patch->signatures[6].foundPtr;
	void* uiLegalsScreen	= patch->signatures[7].foundPtr;
	void* fov				= patch->signatures[8].foundPtr;

	// Find the engine object pointer
	if (!MemRead(enginePtr, &ppEngine, sizeof(ppEngine)))	return false;

	// Remove black bars
	BYTE jmp = 0xEB;
	if (!MemWrite(blackBars,			&jmp, sizeof(jmp)))				return false;
	if (!MemWrite(blackBarsOnResize,	&jmp, sizeof(jmp)))				return false;

	// Fix scaling of UI layers
	static void* pAtoi = &Hook_Atoi;
	if (!MemWriteHookCallPtr(uiSizeHook, &pAtoi))						return false;

	static double* pUiWidth = &uiWidth;
	static double* pUiHeight = &uiHeight;

	void* sizeWidth = (BYTE*)uiSizeHook + 35;
	if (!MemWrite(sizeWidth, &pUiWidth, sizeof(pUiWidth)))				return false;

	if (!isAlternate)
	{
		void* layerWidth = (BYTE*)uiLayerSize + 4;
		void* layerHeight = (BYTE*)uiLayerSize + 43;
		if (!MemWrite(layerWidth, &pUiWidth, sizeof(pUiWidth)))			return false;
		if (!MemWrite(layerHeight, &pUiHeight, sizeof(pUiHeight)))		return false;

		void* layerWidthAlt = (BYTE*)uiLayerSize2 + 4;
		void* layerHeightAlt = (BYTE*)uiLayerSize2 + 38;
		if (!MemWrite(layerWidthAlt, &pUiWidth, sizeof(pUiWidth)))		return false;
		if (!MemWrite(layerHeightAlt, &pUiHeight, sizeof(pUiHeight)))	return false;
	}
	else
	{
		void* layerWidth = (BYTE*)uiLayerSize + 2;
		void* layerHeight = (BYTE*)uiLayerSize + 19;
		if (!MemWrite(layerWidth, &pUiWidth, sizeof(pUiWidth)))			return false;
		if (!MemWrite(layerHeight, &pUiHeight, sizeof(pUiHeight)))		return false;
	}

	void* subtitleWidth = (BYTE*)uiSubtitleLayer + 2;
	if (!MemWrite(subtitleWidth, &pUiWidth, sizeof(pUiWidth)))			return false;

	void* legalsWidth = (BYTE*)uiLegalsScreen + 2;
	if (!MemWrite(legalsWidth, &pUiWidth, sizeof(pUiWidth)))			return false;

	// Correct FoV
	BYTE fovHook[] =
	{
		0x89, 0xD9,							// mov ecx, ebx
		0x51,								// push ecx
		0xE8, MASK, MASK, MASK, MASK,		// call Hook_Fov
		0x8B, 0x17,							// mov edx,[edi]
		0x8B, 0x82, 0xA0, 0x00, 0x00, 0x00,	// mov eax,[edx + 000000A0]
		0xE9, MASK, MASK, MASK, MASK		// jmp $hook
	};

	BYTE* pFovHook = (BYTE*)ExecCopy(fovHook, sizeof(fovHook));

	DWORD* a1 = (DWORD*)&pFovHook[4];
	DWORD* a2 = (DWORD*)&pFovHook[17];

	*a1 = (DWORD)&Hook_Fov - (DWORD)a1 - 4;
	*a2 = (DWORD)fov - (DWORD)a2 + 1; // FIXME: is this correct?

	if (!MemWriteHookJmp(fov, pFovHook))	return false;
	if (!MemWriteNop((BYTE*)fov + 5, 3))	return false;


	//MemWrite((void*)0x00491FD3, &pUiWidth, sizeof(pUiWidth));
	//MemWrite((void*)0x00491FF5, &pUiHeight, sizeof(pUiHeight));	
	//MemWrite((void*)0x00493326, &pUiWidth, sizeof(pUiWidth));
	//MemWrite((void*)0x00493352, &pUiHeight, sizeof(pUiHeight));

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

double fovMultiplier = 1.0f;

// FIXME: blending on fight looks weird
// FIXME: Check if its called from the correct location only once
void __stdcall Hook_Fov(CameraManager* manager)
{
	if (!ppEngine || !manager || !manager->activeCamera) // Safe-guard
	{
		return;
	}

	I3DEngine* engine = *ppEngine;
	assert(engine);

	// Disable FoV correction in cutscenes
	if (manager->activeCamera == manager->cutsceneCamera)
	{
		return;
	}

	double aspect = engine->viewWidth / (double)engine->viewHeight;
	double multiplier = aspect / 1.77777778;

	if (multiplier < 1.0)
	{
		multiplier = 1.0; // If the aspect is smaller than 16:9 => dont do correction
	}

	multiplier *= fovMultiplier;

	manager->activeCamera->fov *= multiplier;
}
