//-----------------------------------------------------------------------------
// lanConstants.h
// 
// Releases:
//     1.0 - Initial release
//
// Copyright (c) 2021 Václav AKA Vaana
//-----------------------------------------------------------------------------

#pragma once

//
// Types, macros and definitions
//

typedef unsigned long Address;
typedef unsigned char Opcode;

#define NO_SECURITY_CHECKS __declspec(safebuffers)

//
// Version info
//

enum LAN_VERSION
{
	LAN_VERSION_2617, // Old Steam version
	/*LAN_VERSION_2651, // Old R* Launcher*/
	LAN_VERSION_2663R, //  R* Launcher
	LAN_VERSION_2663S, // Steam

	LAN_NUM_VERSIONS
};

#define LAN_TIMESTAMP_2617  0x4FF45207 // Old Steam
#define LAN_TIMESTAMP_2663R 0x5FD887BA // R* Launcher
#define LAN_TIMESTAMP_2663S 0x5FF71F4D // Steam

//
// Constants
//

#define LAN_DEFAULT_WIDTH				1280
#define LAN_DEFAULT_HEIGHT				720
#define LAN_DEFAULT_ASPECT_INVERSE		0.5625f

#define LAN_MINIMAL_FRAMERATE			29.97
#define LAN_DEFAULT_BRAKING_CONSTANT	30.00

//
// Addresses
//

enum
{
	OFFSET_POINTER_RENDERER,			// Static pointer to the renderer instance
	OFFSET_HOOK_FRAME,					// Call of function determening the type of the current frame
	OFFSET_PATCH_CAR_BRAKE_CONSTANT,	// Constant affecting the braking speed/force of cars
	OFFSET_PATCH_ASPECT_CHECK,			// Condition checking the aspect ratio and forcing viewport cropping
	OFFSET_HOOK_FIELD_OF_VIEW_VTABLE,	// Vtable of camera class, derived class used in gameplay.
	OFFSET_HOOK_CREATE_WINDOW,			// Call of CreateWindowExA used to create a borderless window
	OFFSET_PATCH_WINDOWED,				// Condition checking if -windowed is set
	OFFSET_HOOK_RESOLUTION,				// Call of atoi used to serialize resolution string

	NUM_OFFSETS
};

const unsigned long addresses[LAN_NUM_VERSIONS][NUM_OFFSETS] =
{
	// 2167
	{
		/* 0x0151AC40 */ 0x0111AC40, // OFFSET_POINTER_RENDERER
		/* 0x00A8E71F */ 0x0068E71F, // OFFSET_FRAME_HOOK
		/* 0x00E4937B */ 0x00A4937B, // OFFSET_PATCH_CAR_BRAKE_CONSTANT
		/* 0x00D51D59 */ 0x00951D59, // OFFSET_PATCH_ASPECT_CHECK
		/* 0x01176268 */ 0x00D76268, // OFFSET_HOOK_FIELD_OF_VIEW_VTABLE
		/* 0x00A914A7 */ 0x006914A7, // OFFSET_HOOK_CREATE_WINDOW
		/* 0x00A89B0A */ 0x00689B0A, // OFFSET_PATCH_WINDOWED
		/* 0x009CF41E */ 0x005CF41E, // OFFSET_HOOK_RESOLUTION
	},

	// 2663 (R* Launcher)
	{
		/* 0x014ED230 */ 0x010ED230, // OFFSET_POINTER_RENDERER
		/* 0x009DBF0F */ 0x005DBF0F, // OFFSET_FRAME_HOOK
		/* 0x00E1C96F */ 0x00A1C96F, // OFFSET_PATCH_CAR_BRAKE_CONSTANT
		/* 0x00D34DD0 */ 0x00934DD0, // OFFSET_PATCH_ASPECT_CHECK
		/* 0x0112F3A0 */ 0x00D2F3A0, // OFFSET_HOOK_FIELD_OF_VIEW_VTABLE
		/* 0x009DEB0E */ 0x005DEB0E, // OFFSET_HOOK_CREATE_WINDOW
		/* 0x009D72A3 */ 0x005D72A3, // OFFSET_PATCH_WINDOWED
		/* 0x0091EB5E */ 0x0051EB5E, // OFFSET_HOOK_RESOLUTION
	},

	// 2663 (Steam)
	{
		/* 0x01524420 */ 0x01124420, // OFFSET_POINTER_RENDERER
		/* 0x009ED68F */ 0x005ED68F, // OFFSET_FRAME_HOOK
		/* 0x00E5186B */ 0x00A5186B, // OFFSET_PATCH_CAR_BRAKE_CONSTANT
		/* 0x00D58BE9 */ 0x00958BE9, // OFFSET_PATCH_ASPECT_CHECK
		/* 0x011663D8 */ 0x00D663D8, // OFFSET_HOOK_FIELD_OF_VIEW_VTABLE
		/* 0x009F04CE */ 0x005F04CE, // OFFSET_HOOK_CREATE_WINDOW
		/* 0x009E8A63 */ 0x005E8A63, // OFFSET_PATCH_WINDOWED
		/* 0x0092DC7E */ 0x0052DC7E, // OFFSET_HOOK_RESOLUTION
	}
};

//
// Helper structs
//

#pragma pack(push, 1)

struct mov
{
	Opcode opcode;
	unsigned char reg;
	Address address;
};

struct mulsd
{
	Opcode opcode;
	unsigned char type1;
	unsigned char type2;
	unsigned char reg;
	Address address;
};

struct fmul
{
	Opcode opcode;
	unsigned char type;
	Address address;
};

struct call
{
	Opcode opcode;
	Address address;
};

struct callPtr
{
	Opcode opcode;
	unsigned char type;
	Address address;
};

struct jmpByte
{
	Opcode opcode;
	unsigned char offset;
};

#pragma pack(pop)

