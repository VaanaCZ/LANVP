// ----------------------------------------------------------------------------
// shared.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

// Helper classes

class I3DEngine
{
public:
	byte padding[0xE8];
	float framerate;
	byte padding2[0xE8];
	int viewWidth;
	int viewHeight;
};

// Common signatures

#define MASK 0xFF

#define MSK1 0xFA

static byte sig_engineDestructor[] =
{
	0x8B, 0x96, 0x9C, 0x00, 0x00, 0x00,
	0x83, 0xE1, 0xF8,
	0x8B, 0x01,
	0x8B, 0x40, 0x0C,
	0x52,
	0xFF, 0xD0,
	0xC7, 0x05, MSK1, MSK1, MSK1, MSK1, 0x00, 0x00, 0x00, 0x00,
	0x8B, 0x4C, 0x24, 0x10,
	0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00
};

#define REGISTER_ENGINE_MASK(p)	REGISTER_MASK(p, sig_engineDestructor, MSK1, 19);

