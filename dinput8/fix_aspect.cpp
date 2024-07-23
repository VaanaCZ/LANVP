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
		0x0F, 0x29, 0x86, 0xD0, 0x00, 0x00, 0x00,
		0xE8, MASK, MASK, MASK, MASK,
		0xD9, 0x47, 0x54,
HERE,	0xD9, 0x9E, 0xE0, 0x00, 0x00, 0x00,
		0xD9, 0x47, 0x58,
		0xD9, 0x9E, 0xE8, 0x00, 0x00, 0x00,
		0x8A, 0x57, 0x64
};

DWORD sigCutsceneCameraVftPtr[] =
{
		0x88, 0x99, 0x80, 0x00, 0x00, 0x00,
		0xF3, 0x0F, 0x11, 0x81, 0x88, 0x00, 0x00, 0x00,
		0xC7, 0x81, 0x8C, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
HERE,	0xC7, 0x01, MASK, MASK, MASK, MASK,
		0x88, 0x99, 0x90, 0x00, 0x00, 0x00,
		0x89, 0x99, 0x84, 0x00, 0x00, 0x00,
		0xEB, MASK
};

DWORD sigAltCutsceneCameraVftPtr[] =
{
		0xC6, 0x81, 0x80, 0x00, 0x00, 0x00, 0x00,
		0xF3, 0x0F, 0x11, 0x81, 0x88, 0x00, 0x00, 0x00,
		0xC7, 0x81, 0x8C, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
HERE,	0xC7, 0x01, MASK, MASK, MASK, MASK,
		0xC6, 0x81, 0x90, 0x00, 0x00, 0x00, 0x00,
		0xC7, 0x81, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xEB, MASK
};

static int enginePtrIndex = -1;
static int blackBarsIndex = -1;
static int blackBarsOnResizeIndex = -1;
static int uiSizeHookIndex = -1;
static int uiLayerSizeIndex = -1;
static int uiLayerSize2Index = -1;
static int uiSubtitleLayerIndex = -1;
static int uiLegalsScreenIndex = -1;
static int fovIndex = -1;
static int cutsceneCameraVftPtrIndex = -1;

void RegisterPatch_Aspect()
{
	Patch patch;

	enginePtrIndex				= patch.AddSignature(SIGARG(sigEnginePtr));
	blackBarsIndex				= patch.AddSignature(SIGARG(sigBlackBars));
	blackBarsOnResizeIndex		= patch.AddSignature(SIGARG(sigBlackBarsOnResize));
	uiSizeHookIndex				= patch.AddSignature(SIGARG(sigUiSizeHook));
	uiLayerSizeIndex			= patch.AddSignatureWithAlt(SIGARG(sigUiLayerSize), SIGARG(sigAltUiLayerSize));
	uiLayerSize2Index			= patch.AddSignatureWithAlt(SIGARG(sigUiLayerSize2), SIGARG(sigAltUiLayerSize));
	uiSubtitleLayerIndex		= patch.AddSignature(SIGARG(sigUiSubtitleLayer));
	uiLegalsScreenIndex			= patch.AddSignature(SIGARG(sigUiLegalsScreen));
	fovIndex					= patch.AddSignature(SIGARG(sigFov));
	cutsceneCameraVftPtrIndex	= patch.AddSignatureWithAlt(SIGARG(sigCutsceneCameraVftPtr), SIGARG(sigAltCutsceneCameraVftPtr));

	patch.SetName(L"Aspect-ratio fix");
	patch.func = ApplyPatch_Aspect;

	RegisterPatch(patch);
}

static I3DEngine** ppEngine;					// Pointer to engine object

static double uiWidth = 1280.0;
static double uiHeight = 720.0;

static void* cutsceneCameraVft;					// CutsceneCamera::__vftptr

bool ApplyPatch_Aspect(Patch* patch)
{
	assert(patch->numSignatureIndices == 9);
	void* enginePtr					= (BYTE*)patch->GetSignature(enginePtrIndex) + 2;
	void* blackBars					= patch->GetSignature(blackBarsIndex);
	void* blackBarsOnResize			= patch->GetSignature(blackBarsOnResizeIndex);
	void* uiSizeHook				= patch->GetSignature(uiSizeHookIndex);
	bool isUiLayerAlt				= false;
	void* uiLayerSize				= patch->GetSignature(uiLayerSizeIndex, &isUiLayerAlt);
	void* uiLayerSize2				= patch->GetSignature(uiLayerSize2Index);
	void* uiSubtitleLayer			= patch->GetSignature(uiSubtitleLayerIndex);
	void* uiLegalsScreen			= patch->GetSignature(uiLegalsScreenIndex);
	void* fov						= patch->GetSignature(fovIndex);
	void* cutsceneCameraVftPtr		= (BYTE*)patch->GetSignature(cutsceneCameraVftPtrIndex) + 2;

	// Find the engine object pointer
	if (!MemRead(enginePtr, &ppEngine, sizeof(ppEngine)))				return false;

	//
	// BLACK BARS
	// 
	// Aspect ratios smaller than 16:9 have black bars on the top
	// and bottom of the screen.
	// Removing this is as simple as jumping over the logic which
	// trims the viewport.
	//

	BYTE jmp = 0xEB;
	if (!MemWrite(blackBars,			&jmp, sizeof(jmp)))				return false;
	if (!MemWrite(blackBarsOnResize,	&jmp, sizeof(jmp)))				return false;

	//
	// UI SCALING
	//
	// On anything other than 16:9, the UI will correctly fill
	// the screen or might appear unreasonably large.
	// This step adjust the values used for sizing UI layers
	// so that they always are correct to the current screen's
	// aspect.
	//

	static void* pAtoi = &Hook_Atoi;
	if (!MemWriteHookCallPtr(uiSizeHook, &pAtoi))						return false;

	static double* pUiWidth = &uiWidth;
	static double* pUiHeight = &uiHeight;

	void* sizeWidth = (BYTE*)uiSizeHook + 35;
	if (!MemWrite(sizeWidth, &pUiWidth, sizeof(pUiWidth)))				return false;

	if (!isUiLayerAlt)
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


	//
	// FOV CORRECTION
	//
	// By default, the game stores the FoV as the horizontal angle
	// across the frustrum in radians. For 16:9 resolutions this works
	// fine, but when the screen becomes wider, the vertical angle
	// becomes smaller, leading to an overall decreased FoV.
	// The correction simply scales the horizontal angle according
	// to the ratio between the current aspect and 16:9, which
	// ensures it's correct all the time.
	// In addition, special care must be taken for cutscenes, since
	// those are in a cinematic 2.40:1 aspect ratio.
	//

	// Find the addresses of the required virtual tables
	if (!MemRead(cutsceneCameraVftPtr, &cutsceneCameraVft, sizeof(void*)))	return false;

	BYTE fovHook[] =
	{
		0xD9, 0x9E, 0xE0, 0x00, 0x00, 0x00,	// fstp dword ptr [esi+000000E0]
		0x89, 0xF9,							// mov ecx, edi
		0x83, 0xE9, 0x10,					// sub ecx, 0x10
		0x56,								// push esi
		0x51,								// push ecx
		0xE8, MASK, MASK, MASK, MASK,		// call Hook_Fov
		0xE9, MASK, MASK, MASK, MASK		// jmp $hook
	};

	BYTE* pFovHook = (BYTE*)ExecCopy(fovHook, sizeof(fovHook));

	if (!MemWriteHookCall(&pFovHook[13], &Hook_Fov))		return false;
	if (!MemWriteHookJmp(&pFovHook[18], (BYTE*)fov + 6))	return false;

	if (!MemWriteHookJmp(fov, pFovHook))					return false;
	if (!MemWriteNop((BYTE*)fov + 5, 1))					return false;

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

// FIXME: Check if its called from the correct location only once
void __stdcall Hook_Fov(ICamera* cameraModifier, Camera* camera)
{
	if (!ppEngine || !cameraModifier || !camera) // Safe-guard
	{
		return;
	}

	I3DEngine* engine = *ppEngine;
	assert(engine);

	double aspect = engine->viewWidth / (double)engine->viewHeight;

	// Disable FoV correction in cutscenes
	if (cameraModifier->__vftptr == cutsceneCameraVft)
	{
		double cutsceneMultiplier = aspect / 2.4;

		if (cutsceneMultiplier > 1.0)
		{
			camera->fov *= cutsceneMultiplier;
		}

		return;
	}

	double multiplier = aspect / 1.77777778;

	if (multiplier < 1.0)
	{
		multiplier = 1.0; // If the aspect is smaller than 16:9 => don't do correction
	}

	multiplier *= fovMultiplier;

	camera->fov *= multiplier;
}
