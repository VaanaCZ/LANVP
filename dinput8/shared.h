// ----------------------------------------------------------------------------
// shared.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

#define VPATCH_VERSION_MAJOR "2"
#define VPATCH_VERSION_MINOR "0"
#define VPATCH_VERSION_PATCH ""

#define VPATCH_VERSION		VPATCH_VERSION_MAJOR "." VPATCH_VERSION_MINOR VPATCH_VERSION_PATCH
#define VPATCH_VERSION_MENU VPATCH_VERSION_MAJOR "·" VPATCH_VERSION_MINOR VPATCH_VERSION_PATCH

// Helper classes

class I3DEngine
{
public:
	BYTE padding[0xE8];
	float framerate;
	BYTE padding2[0xE8];
	int viewWidth;
	int viewHeight;
};

// Common signatures

#define MASK 0xFFFFFFFF // fixme
#define HERE 0xDDDDDDDD // fixme

static DWORD sigEngineDestructor[] =
{
		0x8B, 0x96, 0x9C, 0x00, 0x00, 0x00,
		0x83, 0xE1, 0xF8,
		0x8B, 0x01,
		0x8B, 0x40, 0x0C,
		0x52,
		0xFF, 0xD0,
		0xC7, 0x05, HERE, MASK, MASK, MASK, MASK, 0x00, 0x00, 0x00, 0x00,
		0x8B, 0x4C, 0x24, 0x10,
		0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00
};

#define REGISTER_ENGINE_MASK(p)	REGISTER_MASK(p, sigEngineDestructor);
