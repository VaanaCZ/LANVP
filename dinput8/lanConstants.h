//-----------------------------------------------------------------------------
// lanConstants.h
// 
// Releases:
//     1.0  - Initial release
//     1.1  - "FPS Unlock" & "Aspect Correction" improvements, "Launcher Check",
//            "Skip Logo&Legals" & "FPS Lock" added, "Force Resolution" bugfix.
//     1.1a - Added "Force DX11" option, fixed a bug with force resolution.
//
// Copyright (c) 2021 Václav AKA Vaana
//-----------------------------------------------------------------------------

#pragma once

#define VPATCH_VERSION_MAJOR "1"
#define VPATCH_VERSION_MINOR "1a"
#define VPATCH_VERSION_PATCH ""

#define VPATCH_VERSION		VPATCH_VERSION_MAJOR "." VPATCH_VERSION_MINOR VPATCH_VERSION_PATCH
#define VPATCH_VERSION_MENU VPATCH_VERSION_MAJOR "·" VPATCH_VERSION_MINOR VPATCH_VERSION_PATCH

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
	LAN_VERSION_2617R, // Old retail version
	LAN_VERSION_2617S, // Old Steam version
	/*LAN_VERSION_2651, // Old R* Launcher*/
	LAN_VERSION_2663R, //  R* Launcher
	LAN_VERSION_2663S, // Steam

	LAN_NUM_VERSIONS
};

#define LAN_LATEST_BUILD	2663

#define LAN_TIMESTAMP_2617R 0x4FF2D605 // Old retail
#define LAN_TIMESTAMP_2617S 0x4FF45207 // Old Steam
#define LAN_TIMESTAMP_2663R 0x5FD887BA // R* Launcher
#define LAN_TIMESTAMP_2663S 0x5FF71F4D // Steam

//
// Constants
//

#define LAN_DEFAULT_WIDTH				1280
#define LAN_DEFAULT_HEIGHT				720
#define LAN_DEFAULT_ASPECT				1.77777778f
#define LAN_DEFAULT_ASPECT_INVERSE		0.5625f

#define LAN_MINIMAL_FRAMERATE			29.97
#define LAN_DEFAULT_BRAKING_CONSTANT	30.00
#define LAN_ONE_OVER_FRAMERATE			0.01668335f

//
// Addresses
//

enum
{
	OFFSET_POINTER_RENDERER,			// Static pointer to the renderer instance.
	OFFSET_HOOK_VERSION,				// Call of VerQueryValue used to determine the current game version.
	OFFSET_HOOK_FRAME,					// Call of function determening the type of the current frame.
	OFFSET_HOOK_MAP_VTABLE,				// Vtable entry of map, used for input processing.
	OFFSET_PATCH_CAR_BRAKE_CONSTANT,	// Constant affecting the braking speed/force of cars.
	OFFSET_PATCH_ASPECT_FILTER,			// Condition filtering out unsupported ratios from the resolution list.
	OFFSET_PATCH_ASPECT_CROP,			// Condition checking the aspect ratio and forcing viewport cropping.
	OFFSET_HOOK_FIELD_OF_VIEW_VTABLE,	// Vtable of camera class, derived class used in gameplay.
	OFFSET_HOOK_INTERFACE,				// Call of atoi used to parse the resolution string.
	OFFSET_VALUE_INTERFACE_WIDTH,		// Virtual interface width
	OFFSET_VALUE_INTERFACE_HEIGHT,		// Virtual interface height
	OFFSET_PATCH_LAUNCHER_CHECK,		// Condition checking whether the game was launched through the launcher.
	OFFSET_PATCH_MOVIE_LOGO,			// Call of function to load and play the intro logo.
	OFFSET_PATCH_LEGALS_SCREEN,			// Call of function to show the legals screen.
	OFFSET_PATCH_DX11,					// Condition checking if -dx11 is set.
	OFFSET_PATCH_ARGS_RESOLUTION,		// Condition checking if -res is set.
	OFFSET_HOOK_RESOLUTION,				// Call of strtol to parse the resolution argument.
	OFFSET_PATCH_RESOLUTION_CHECK,		// Condition checking if the resolution is in the resolution list.
	OFFSET_HOOK_CREATE_WINDOW,			// Call of CreateWindowExA used to create a borderless window.
	OFFSET_PATCH_ARGS_WINDOWED,			// Condition checking if -windowed is set.

	NUM_OFFSETS
};

const Address addresses[LAN_NUM_VERSIONS][NUM_OFFSETS] =
{
	// 2617 (retail)
	{
		/* 0x014EBC30 */ 0x010EBC30, // OFFSET_POINTER_RENDERER
		/* 0x0041F92D */ 0x0001F92D, // OFFSET_HOOK_VERSION
		/* 0x00A7D26F */ 0x0067D26F, // OFFSET_HOOK_FRAME
		/* 0x010D1C60 */ 0x00CD1C60, // OFFSET_HOOK_MAP_VTABLE
		/* 0x00E1B17F */ 0x00A1B17F, // OFFSET_PATCH_CAR_BRAKE_CONSTANT
		/* 0x009BFC82 */ 0x005BFC82, // OFFSET_PATCH_ASPECT_FILTER
		/* 0x00D34E30 */ 0x00934E30, // OFFSET_PATCH_ASPECT_CROP
		/* 0x01146380 */ 0x00D46380, // OFFSET_HOOK_FIELD_OF_VIEW_VTABLE
		/* 0x009C119D */ 0x005C119D, // OFFSET_HOOK_INTERFACE
		/* 0x0122A0C8 */ 0x00E2A0C8, // OFFSET_VALUE_INTERFACE_WIDTH
		/* 0x0122A0C0 */ 0x00E2A0C0, // OFFSET_VALUE_INTERFACE_HEIGHT
		/* 0x00A7797E */ 0x0067797E, // OFFSET_PATCH_LAUNCHER_CHECK
		/* 0x00A7B48C */ 0x0067B48C, // OFFSET_PATCH_MOVIE_LOGO
		/* 0x0103AC70 */ 0x00C3AC70, // OFFSET_PATCH_LEGALS_SCREEN
		/* 0x00A77BBC */ 0x00677BBC, // OFFSET_PATCH_DX11
		/* 0x00A7869A */ 0x0067869A, // OFFSET_PATCH_ARGS_RESOLUTION
		/* 0x00A786E1 */ 0x006786E1, // OFFSET_HOOK_RESOLUTION
		/* 0x00B04EEF */ 0x00704EEF, // OFFSET_PATCH_RESOLUTION_CHECK
		/* 0x00A7FE77 */ 0x0067FE77, // OFFSET_HOOK_CREATE_WINDOW
		/* 0x00A7867C */ 0x0067867C, // OFFSET_PATCH_ARGS_WINDOWED
	},

	// 2617 (Steam)
	{
		/* 0x0151AC40 */ 0x0111AC40, // OFFSET_POINTER_RENDERER
		/* 0x0041FBA0 */ 0x0001FBA0, // OFFSET_HOOK_VERSION
		/* 0x00A8E71F */ 0x0068E71F, // OFFSET_HOOK_FRAME
		/* 0x01101B38 */ 0x00D01B38, // OFFSET_HOOK_MAP_VTABLE
		/* 0x00E4937B */ 0x00A4937B, // OFFSET_PATCH_CAR_BRAKE_CONSTANT
		/* 0x009CE822 */ 0x005CE822, // OFFSET_PATCH_ASPECT_FILTER
		/* 0x00D51D59 */ 0x00951D59, // OFFSET_PATCH_ASPECT_CROP
		/* 0x01176268 */ 0x00D76268, // OFFSET_HOOK_FIELD_OF_VIEW_VTABLE
		/* 0x009CFD3D */ 0x005CFD3D, // OFFSET_HOOK_INTERFACE
		/* 0x01259E50 */ 0x00E59E50, // OFFSET_VALUE_INTERFACE_WIDTH
		/* 0x01259E48 */ 0x00E59E48, // OFFSET_VALUE_INTERFACE_HEIGHT
		/* 0x00A88E1E */ 0x00688E1E, // OFFSET_PATCH_LAUNCHER_CHECK
		/* 0x00A8C93E */ 0x0068C93E, // OFFSET_PATCH_MOVIE_LOGO
		/* 0x0106ADC0 */ 0x00C6ADC0, // OFFSET_PATCH_LEGALS_SCREEN
		/* 0x00A890B6 */ 0x006890B6, // OFFSET_PATCH_DX11
		/* 0x00A89B28 */ 0x00689B28, // OFFSET_PATCH_ARGS_RESOLUTION
		/* 0x00A89B6F */ 0x00689B6F, // OFFSET_HOOK_RESOLUTION
		/* 0x00B17D3F */ 0x00717D3F, // OFFSET_PATCH_RESOLUTION_CHECK
		/* 0x00A914A7 */ 0x006914A7, // OFFSET_HOOK_CREATE_WINDOW
		/* 0x00A89B0A */ 0x00689B0A, // OFFSET_PATCH_ARGS_WINDOWED
	},

	// 2663 (R* Launcher)
	{
		/* 0x014ED230 */ 0x010ED230, // OFFSET_POINTER_RENDERER
		/* 0x00B8B99A */ 0x0078B99A, // OFFSET_HOOK_VERSION
		/* 0x009DBF0F */ 0x005DBF0F, // OFFSET_HOOK_FRAME
		/* 0x011FAC78 */ 0x00DFAC78, // OFFSET_HOOK_MAP_VTABLE
		/* 0x00E1C96F */ 0x00A1C96F, // OFFSET_PATCH_CAR_BRAKE_CONSTANT
		/* 0x0091DF62 */ 0x0051DF62, // OFFSET_PATCH_ASPECT_FILTER
		/* 0x00D34DD0 */ 0x00934DD0, // OFFSET_PATCH_ASPECT_CROP
		/* 0x0112F3A0 */ 0x00D2F3A0, // OFFSET_HOOK_FIELD_OF_VIEW_VTABLE
		/* 0x0091F47D */ 0x0051F47D, // OFFSET_HOOK_INTERFACE
		/* 0x0122BB10 */ 0x00E2BB10, // OFFSET_VALUE_INTERFACE_WIDTH
		/* 0x0122BB08 */ 0x00E2BB08, // OFFSET_VALUE_INTERFACE_HEIGHT
		/* 0x009D666C */ 0x005D666C, // OFFSET_PATCH_LAUNCHER_CHECK
		/* 0x009DA0F6 */ 0x005DA0F6, // OFFSET_PATCH_MOVIE_LOGO
		/* 0x01066B30 */ 0x00C66B30, // OFFSET_PATCH_LEGALS_SCREEN
		/* 0x009D686D */ 0x005D686D, // OFFSET_PATCH_DX11
		/* 0x009D72C1 */ 0x005D72C1, // OFFSET_PATCH_ARGS_RESOLUTION
		/* 0x009D7308 */ 0x005D7308, // OFFSET_HOOK_RESOLUTION
		/* 0x00A6408F */ 0x0066408F, // OFFSET_PATCH_RESOLUTION_CHECK
		/* 0x009DEB0E */ 0x005DEB0E, // OFFSET_HOOK_CREATE_WINDOW
		/* 0x009D72A3 */ 0x005D72A3, // OFFSET_PATCH_ARGS_WINDOWED
	},

	// 2663 (Steam)
	{
		/* 0x01524420 */ 0x01124420, // OFFSET_POINTER_RENDERER
		/* 0x00BA2F00 */ 0x007A2F00, // OFFSET_HOOK_VERSION
		/* 0x009ED68F */ 0x005ED68F, // OFFSET_HOOK_FRAME
		/* 0x01231CD8 */ 0x00E31CD8, // OFFSET_HOOK_MAP_VTABLE
		/* 0x00E5186B */ 0x00A5186B, // OFFSET_PATCH_CAR_BRAKE_CONSTANT
		/* 0x0092D082 */ 0x0052D082, // OFFSET_PATCH_ASPECT_FILTER
		/* 0x00D58BE9 */ 0x00958BE9, // OFFSET_PATCH_ASPECT_CROP
		/* 0x011663D8 */ 0x00D663D8, // OFFSET_HOOK_FIELD_OF_VIEW_VTABLE
		/* 0x0092E59D */ 0x0052E59D, // OFFSET_HOOK_INTERFACE
		/* 0x01262C00 */ 0x00E62C00, // OFFSET_VALUE_INTERFACE_WIDTH
		/* 0x01262BF8 */ 0x00E62BF8, // OFFSET_VALUE_INTERFACE_HEIGHT
		/* 0x009E7DCC */ 0x005E7DCC, // OFFSET_PATCH_LAUNCHER_CHECK
		/* 0x009EB8B6 */ 0x005EB8B6, // OFFSET_PATCH_MOVIE_LOGO
		/* 0x0109DED0 */ 0x00C9DED0, // OFFSET_PATCH_LEGALS_SCREEN
		/* 0x009E8025 */ 0x005E8025, // OFFSET_PATCH_DX11
		/* 0x009E8A81 */ 0x005E8A81, // OFFSET_PATCH_ARGS_RESOLUTION
		/* 0x009E8AC8 */ 0x005E8AC8, // OFFSET_HOOK_RESOLUTION
		/* 0x00A775FF */ 0x006775FF, // OFFSET_PATCH_RESOLUTION_CHECK
		/* 0x009F04CE */ 0x005F04CE, // OFFSET_HOOK_CREATE_WINDOW
		/* 0x009E8A63 */ 0x005E8A63, // OFFSET_PATCH_ARGS_WINDOWED
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

struct add
{
	Opcode opcode;
	unsigned char reg;
	unsigned char value;
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

struct fdiv
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

struct retNear
{
	Opcode opcode;
};

#pragma pack(pop)

