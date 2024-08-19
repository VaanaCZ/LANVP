// ----------------------------------------------------------------------------
// fix_fps.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "fix_fps.h"

#include "patching.h"
#include "shared.h"
#include <cassert>

DWORD sigFramerateDivisorConstructor[] =
{
		0x89, 0x5C, 0x24, 0x18,
		0x64, 0x8B, 0x15, 0x2C, 0x00, 0x00, 0x00,
		0xF3, 0x0F, 0x10, 0x05, MASK, MASK, MASK, MASK,
HERE,	0xC7, 0x00, 0x02, 0x00, 0x00, 0x00,
		0x8B, 0x02,
		0xC7, 0x06, MASK, MASK, MASK, MASK
};

DWORD sigAltFramerateDivisorConstructor[] =
{
		0x89, 0x5C, 0x24, 0x18,
		0xD9, 0xE8,
		0x64, 0x8B, 0x15, 0x2C, 0x00, 0x00, 0x00,
		0xD9, 0x56, 0x0C,
HERE,	0xC7, 0x00, 0x02, 0x00, 0x00, 0x00,
		0xD9, 0x5E, 0x10,
		0x8B, 0x02,
		0xC7, 0x06, MASK, MASK, MASK, MASK
};

DWORD sigFramerateDivisorGameplay[] =
{
		0xFE, 0x0D, MASK, MASK, MASK, MASK,
		0xFF, 0xD2,
		0x8B, 0x0D, MASK, MASK, MASK, MASK,
HERE,	0xC7, 0x46, 0x04, 0x02, 0x00, 0x00, 0x00,
		0x8B, 0x01,
		0x8B, 0x50, 0x40
};

DWORD sigWait[] =
{
		0x0F, 0x57, 0xC0,
		0x0F, 0x2F, 0x44, 0x24, 0x0C,
HERE,	0x76, MASK,
		0xD9, 0x44, 0x24, 0x0C,
		0xDC, 0x0D, MASK, MASK, MASK, MASK,
		0xD9, 0x7C, 0x24, 0x0C
};

DWORD sigBraking[] =
{
		0x0F, 0xC6, 0xC0, 0x00,
		0x0F, 0x59, 0xC1,
		0xE8, MASK, MASK, MASK, MASK,
HERE,	0xF3, 0x0F, 0x10, 0x45, 0x08,
		0x0F, 0x28, 0x4C, 0x24, 0x40,
		0x0F, 0x5A, 0xC0
};

DWORD sigAltBraking[] =
{
		0x0F, 0xC6, 0xC0, 0x00,
		0x0F, 0x59, 0xC1,
		0xE8, MASK, MASK, MASK, MASK,
HERE,	0xD9, 0x45, 0x08,
		0xDC, 0x0D, MASK, MASK, MASK, MASK,
		0x8D, 0x94, 0x24, 0xA0, 0x00, 0x00, 0x00
};

DWORD sigPencil[] =
{
		0x8B, 0x42, 0x10,
		0x57,
		0x51,
		0xD9, 0x1C, 0x24,
		0xFF, 0xD0,
HERE,	0x8B, 0x4D, 0x04,
		0x8B, 0x41, 0x10,
		0x2B, 0xC6
};

DWORD sigNotebookClueVftPtr[] =
{
		0x56,
		0xE8, MASK, MASK, MASK, MASK,
		0xB8, 0xFF, 0xFF, 0x00, 0x00,
HERE,	0xC7, 0x06, MASK, MASK, MASK, MASK,
		0xC7, 0x46, 0x24, MASK, MASK, MASK, MASK,
		0x66, 0x89, 0x86, 0xB0, 0x00, 0x00, 0x00,
		0x64, 0x8B, 0x0D, 0x2C, 0x00, 0x00, 0x00
};

DWORD sigPairedAnimStageVftPtr[] =
{
		0x89, 0x48, 0x08,
		0xC7, 0x40, 0x0C, MASK, MASK, MASK, MASK,
		0x89, 0x48, 0x10,
HERE,	0xC7, 0x00, MASK, MASK, MASK, MASK,
		0xC7, 0x40, 0x0C, MASK, MASK, MASK, MASK,
		0x89, 0x48, 0x14,
		0x89, 0x48, 0x18
};

DWORD sigBirds[] =
{
		0xF3, 0x0F, 0x11, 0x83, MASK, MASK, MASK, MASK,
		0x77, MASK,
		0x0F, 0x28, 0xC1,
HERE,	0xF3, 0x0F, 0x10, 0x0D, MASK, MASK, MASK, MASK,
		0x0F, 0x2F, 0xC8,
		0x77, MASK,
		0x0F, 0x28, 0xC1,
		0xF3, 0x0F, 0x10, 0x8B, MASK, MASK, MASK, MASK,
		0xF3, 0x0F, 0x10, 0x93, MASK, MASK, MASK, MASK
};

static int enginePtrIndex = -1;
static int framerateDivisorConstructorIndex = -1;
static int framerateDivisorGameplayIndex = -1;
static int waitIndex = -1;
static int brakingIndex = -1;
static int pencilIndex = -1;
static int notebookClueVftPtrIndex = -1;
static int pairedAnimStageVftPtrIndex = -1;
static int birdsIndex = -1;

void RegisterPatch_Framerate()
{
	Patch patch;

	enginePtrIndex						= patch.AddSignature(SIGARG(sigEnginePtr));
	framerateDivisorConstructorIndex	= patch.AddSignatureWithAlt(SIGARG(sigFramerateDivisorConstructor), SIGARG(sigAltFramerateDivisorConstructor));
	framerateDivisorGameplayIndex		= patch.AddSignature(SIGARG(sigFramerateDivisorGameplay));
	waitIndex							= patch.AddSignature(SIGARG(sigWait));
	brakingIndex						= patch.AddSignatureWithAlt(SIGARG(sigBraking), SIGARG(sigAltBraking));
	pencilIndex							= patch.AddSignature(SIGARG(sigPencil));
	notebookClueVftPtrIndex				= patch.AddSignature(SIGARG(sigNotebookClueVftPtr));
	pairedAnimStageVftPtrIndex			= patch.AddSignature(SIGARG(sigPairedAnimStageVftPtr));
	birdsIndex							= patch.AddSignature(SIGARG(sigBirds));

	patch.SetName(L"Framerate Unlock");
	patch.func = &ApplyPatch_Framerate;

	RegisterPatch(patch);
}

static I3DEngine** ppEngine;					// Pointer to engine object
static LARGE_INTEGER lastTime, timeFrequency;	// Time measurement variables

static DWORD fixedFrametime = 0x3D088889;		// Default frametime => 0.03333333507

static void* pairedAnimStageVft;				// PairedAnimStage::__vftptr
static void* notebookClueVft;					// NotebookClue::__vftptr

static float birdMaxSpeed;
static float* defaultBirdMaxSpeed;




static float fixedFrametime2 = 1.0f;		// Default frametime => 0.03333333507



class CarDynamicsInstance
{
public:
	int E576A0(float);
	int E4A020(float);
	int E4AFE0(float);
	void E4C2F0(float);
	void E490B0(float);
	int E4F3A0(float);
	int E524B0(float, char, char);
	void E49830(float, float*);

	typedef int (CarDynamicsInstance::* sub1_t)(float);
	typedef void (CarDynamicsInstance::* sub2_t)(float);
	typedef int (CarDynamicsInstance::* sub3_t)(float, char, char);
	typedef void (CarDynamicsInstance::* sub4_t)(float, float*);
};

static CarDynamicsInstance::sub1_t addressE576A0 = &CarDynamicsInstance::E576A0;
static CarDynamicsInstance::sub1_t addressE4A020 = &CarDynamicsInstance::E4A020;
static CarDynamicsInstance::sub1_t addressE4AFE0 = &CarDynamicsInstance::E4AFE0;
static CarDynamicsInstance::sub2_t addressE4C2F0 = &CarDynamicsInstance::E4C2F0;
static CarDynamicsInstance::sub2_t addressE490B0 = &CarDynamicsInstance::E490B0;
static CarDynamicsInstance::sub1_t addressE4F3A0 = &CarDynamicsInstance::E4F3A0;
static CarDynamicsInstance::sub3_t addressE524B0 = &CarDynamicsInstance::E524B0;
static CarDynamicsInstance::sub4_t addressE49830 = &CarDynamicsInstance::E49830;




/*bool SuspTest(void* ptr)
{
	BYTE suspHook[] =
	{
		0xD9, 0x05, MASK, MASK, MASK, MASK,	// fld dword ptr [$fixedFrametime]
		0xDC, 0x0D, MASK, MASK, MASK, MASK,	// fmul dword ptr [$30]
		0xE9, MASK, MASK, MASK, MASK		// jmp $hook
	};

	BYTE* pSuspHook = (BYTE*)ExecCopy(suspHook, sizeof(suspHook));
	assert(pSuspHook);

	if (!MemRead((BYTE*)ptr + 3, &pSuspHook[6], 6))			return false;

	DWORD* a1 = (DWORD*)&pSuspHook[2];
	*a1 = (DWORD)&fixedFrametime;

	if (!MemWriteHookJmp(&pSuspHook[12], (BYTE*)ptr + 9))	return false;

	if (!MemWriteHookJmp(ptr, pSuspHook))					return false;
	if (!MemWriteNop((BYTE*)ptr + 5, 4))					return false;
}


bool SuspTest2(void* ptr, BYTE regs)
{
	BYTE suspHook[] =
	{
		0xF3, 0x0F, 0x5A, regs, MASK, MASK, MASK, MASK,	// cvtss2sd xmm0, [$fixedFrametime]
		0xE9, MASK, MASK, MASK, MASK					// jmp $hook
	};

	BYTE* pSuspHook = (BYTE*)ExecCopy(suspHook, sizeof(suspHook));
	assert(pSuspHook);

	DWORD* a1 = (DWORD*)&pSuspHook[4];
	*a1 = (DWORD)&fixedFrametime;

	if (!MemWriteHookJmp(&pSuspHook[8], (BYTE*)ptr + 5))	return false;

	if (!MemWriteHookJmp(ptr, pSuspHook))					return false;
}*/


bool ApplyPatch_Framerate(Patch* patch)
{
	assert(patch->numSignatureIndices == 9);
	void* enginePtr						= (BYTE*)patch->GetSignature(enginePtrIndex) + 2;
	void* framerateDivisorConstructor	= (BYTE*)patch->GetSignature(framerateDivisorConstructorIndex) + 2;
	void* framerateDivisorGameplay		= (BYTE*)patch->GetSignature(framerateDivisorGameplayIndex) + 3;
	void* wait							= patch->GetSignature(waitIndex);
	bool isBrakingAlt					= false;
	void* braking						= patch->GetSignature(brakingIndex, &isBrakingAlt);
	void* pencil						= patch->GetSignature(pencilIndex);
	void* notebookClueVftPtr			= (BYTE*)patch->GetSignature(notebookClueVftPtrIndex) + 2;
	void* pairedAnimStageVftPtr			= (BYTE*)patch->GetSignature(pairedAnimStageVftPtrIndex) + 2;
	void* birds							= (BYTE*)patch->GetSignature(birdsIndex) + 4;

	// ========================================================================
	// Framerate cap removal
	// ========================================================================

	// Find the engine object pointer
	if (!MemRead(enginePtr, &ppEngine, sizeof(ppEngine)))	return false;

	//
	// By default, the game has a target framerate of 59.97. However there is
	// a divisor value, which is used to determine the final framerate.
	// In menus, this value is set to 1 (60/1=60), but during gameplay, this
	// is set to 2 (60/2=30).
	// 
	// Here we patch out this divisor in gameplay, since it makes the rest of
	// the patch a lot simpler, and prevents potential bugs.
	//

	static DWORD newFramerateDivisor = 1;
	if (!MemWrite(framerateDivisorConstructor, &newFramerateDivisor, sizeof(newFramerateDivisor)))	return false;
	if (!MemWrite(framerateDivisorGameplay, &newFramerateDivisor, sizeof(newFramerateDivisor)))		return false;

	//
	// In order to fully "uncap" the framerate, we must first allow the game
	// to run at full speed. Here we patch out the waiting logic and hook our
	// own function, which then increases or decreases the speed of the
	// simulation based on how quickly the game runs.
	//

	jmpShort jmp;
	if (!MemRead(wait, &jmp, sizeof(jmp)))				return false;

	if (!MemWriteHookCall(wait, &Hook_Frame))			return false; // replace conditional jump with hook
	jmp.opcode = 0xEB;
	jmp.offset -= 5;
	if (!MemWrite((BYTE*)wait + 5, &jmp, sizeof(jmp)))	return false; // append hook with jump to end of branch

	// ========================================================================
	// FPS-related fixes
	// ========================================================================

	//
	// CAR BRAKING
	// 
	// When driving cars, the game creates forces and impulses based on the
	// user's inputs, which are then passed down to the physics system (Havok)
	// that actually performs the physics simulation. Havok will then correctly
	// scale these forces according to the FPS.
	// 
	// Unfortunately, the force which is used for braking is already scaled
	// by the engine once before it is passed to Havok, meaning it gets scaled twice.
	// This causes the braking to become weaker and weaker, the smaller
	// the frametime (high FPS)
	// 
	// forcePerFrameVector = forceVector * frametime * frametime;
	// 
	// The fix is to simply hardcode one of these multiplications to a fixed 1/30 value,
	// so that the final scaling is correct on FPS above 30.
	// 
	// forcePerFrameVector = forceVector * 0.0333333 * frametime;
	//

	if (!isBrakingAlt)
	{
		BYTE brakeHook[] =
		{
			0xF3, 0x0F, 0x10, 0x05, MASK, MASK, MASK, MASK,	// movss xmm0, dword ptr [$fixedFrametime]
			0xE9, MASK, MASK, MASK, MASK					// jmp $hook
		};

		BYTE* pBrakeHook = (BYTE*)ExecCopy(brakeHook, sizeof(brakeHook));
		assert(pBrakeHook);

		DWORD* a1 = (DWORD*)&pBrakeHook[4];
		*a1 = (DWORD)&fixedFrametime;

		if (!MemWriteHookJmp(&pBrakeHook[8], (BYTE*)braking + 5))	return false;

		if (!MemWriteHookJmp(braking, pBrakeHook))					return false;
	}
	else
	{
		BYTE brakeHook[] =
		{
			0xD9, 0x05, MASK, MASK, MASK, MASK,	// fld dword ptr [$fixedFrametime]
			0xDC, 0x0D, MASK, MASK, MASK, MASK,	// fmul dword ptr [$30]
			0xE9, MASK, MASK, MASK, MASK		// jmp $hook
		};

		BYTE* pBrakeHook = (BYTE*)ExecCopy(brakeHook, sizeof(brakeHook));
		assert(pBrakeHook);

		if (!MemRead((BYTE*)braking + 3, &pBrakeHook[6], 6))			return false;

		DWORD* a1 = (DWORD*)&pBrakeHook[2];
		*a1 = (DWORD)&fixedFrametime;

		if (!MemWriteHookJmp(&pBrakeHook[12], (BYTE*)braking + 9))	return false;

		if (!MemWriteHookJmp(braking, pBrakeHook))					return false;
		if (!MemWriteNop((BYTE*)braking + 5, 4))					return false;
	}

	//
	// PENCIL CLUE
	//
	// When the player picks up the pencil in the case "The Set Up", there
	// are two things which have to happen:
	//   Firstly, a PickupEventHandler should be called.
	//   Secondly, the InspectionSystem should begin the interaction with
	//   the object.
	// 
	// For some odd reason, above 50 FPS these things happen in the
	// incorrect order and the InspectionSystem panics and cancels,
	// since it does not yet have the correct values set by the pickup event.
	// 
	// The best way I have figured out to fix this is to manually delay
	// the InspectionSystem entering the next stage via a hook and
	// cock-blocking the progression until the pickup event happens.
	// 
	// It's probably related to some weird animation event floating point
	// timing fuckery, and I haven't got the slightest fucking clue how to fix it!
	//

	// Find the addresses of the required virtual tables
	if (!MemRead(pairedAnimStageVftPtr, &pairedAnimStageVft,	sizeof(void*)))	return false;
	if (!MemRead(notebookClueVftPtr,	&notebookClueVft,		sizeof(void*)))	return false;

	BYTE pencilHook[] =
	{
		0x89, 0xE9,							// mov ecx, ebp
		0x51,								// push ecx
		0xE8, MASK, MASK, MASK, MASK,		// call $Hook_Pencil
		0x8B, 0x4D, 0x04,					// mov ecx,[ebp + 04]
		0x8B, 0x41, 0x10,					// mov eax,[ecx + 10]
		0xE9, MASK, MASK, MASK, MASK		// jmp $hook
	};

	BYTE* pPencilHook = (BYTE*)ExecCopy(pencilHook, sizeof(pencilHook));

	if (!MemWriteHookCall(&pPencilHook[3], &Hook_Pencil))		return false;
	if (!MemWriteHookJmp(&pPencilHook[14], (BYTE*)pencil + 6))	return false;

	if (!MemWriteHookJmp(pencil, pPencilHook))					return false;
	if (!MemWriteNop((BYTE*)pencil + 5, 1))						return false;

	//
	// PIGEON TAKEOFF
	//
	// Birds have a maximum top speed when taking off. However, this speed
	// is specified per frame (0.01 units/frame) and thus it becomes too
	// large on high FPS.
	// 
	// 0.01 * 30  = 0.3 u/s
	// 0.01 * 160 = 1.6 u/s
	//
	// The fix adjusts the maximum top speed according to the framerate.
	//

	if (!MemRead(birds, &defaultBirdMaxSpeed, sizeof(defaultBirdMaxSpeed)))	return false;
	birdMaxSpeed = *defaultBirdMaxSpeed;

	float* pBirdMaxSpeed = &birdMaxSpeed;
	if (!MemWrite(birds, &pBirdMaxSpeed, sizeof(pBirdMaxSpeed)))			return false;

	// Prepare required variables
	if (!QueryPerformanceCounter(&lastTime))		{ HandleError(L"[V-PATCH] Patching failed!", L"Could not query performance counter."); return false; }
	if (!QueryPerformanceFrequency(&timeFrequency)) { HandleError(L"[V-PATCH] Patching failed!", L"Could not query performance frequency."); return false; }









//	SuspTest((void*)0x00E4C5BB);
//	SuspTest2((void*)0x00E4C5DB, 0x05);
//	SuspTest2((void*)0x00E4C698, 0x25);
//	SuspTest2((void*)0x00E4C6BC, 0x25);




	void* ptr = (void*)0x00E4DF33;

	BYTE suspHook[] =
	{
		0xF3, 0x0F, 0x10, 0x0D, MASK, MASK, MASK, MASK,	// movss xmm1, [$fixedFrametime]
		0xE9, MASK, MASK, MASK, MASK					// jmp $hook
	};

	BYTE* pSuspHook = (BYTE*)ExecCopy(suspHook, sizeof(suspHook));
	assert(pSuspHook);

	DWORD* a1 = (DWORD*)&pSuspHook[4];
	*a1 = (DWORD)&fixedFrametime2;

	if (!MemWriteHookJmp(&pSuspHook[8], (BYTE*)ptr + 6))	return false;

	if (!MemWriteHookJmp(ptr, pSuspHook))					return false;
	if (!MemWriteNop((BYTE*)ptr + 5, 1))					return false;





	void* tempAddr;

#define TEMP_PATCH(addr, func)	\
		tempAddr = (void*)addr;			\
		MemReplace(tempAddr, &address##func, sizeof(address##func))

	//TEMP_PATCH(0x01259AB0, E576A0);
	//TEMP_PATCH(0x01259AB4, E4A020);
	//TEMP_PATCH(0x01259AC4, E4AFE0); // fixes turning somewhat
	////TEMP_PATCH(0x01259AC8, E4C2F0); // supsenison
	//TEMP_PATCH(0x01259ACC, E490B0); // braking
	//TEMP_PATCH(0x01259AD0, E4F3A0);
	//TEMP_PATCH(0x01259AD4, E524B0);
	//TEMP_PATCH(0x01259AE8, E49830);















	return true;
}


static float ftm = 1.0f / 30.0f;

double minFrameTime = 0.0;

void __stdcall Hook_Frame()
{
	I3DEngine* engine = *ppEngine;
	assert(engine);

	LARGE_INTEGER currTime;
	QueryPerformanceCounter(&currTime);

	LONGLONG timeDiff = currTime.QuadPart - lastTime.QuadPart;

	// Frame limiter
	LONGLONG minCounter = timeFrequency.QuadPart * minFrameTime;

	while (timeDiff < minCounter)
	{
		QueryPerformanceCounter(&currTime);
		timeDiff = currTime.QuadPart - lastTime.QuadPart;
	}

	// Adjust game speed
	LONGLONG maxCounter = timeFrequency.QuadPart * 0.04; // 25 FPS (safe limit)
	timeDiff = min(maxCounter, timeDiff);

	double fps = timeFrequency.QuadPart / (double)timeDiff;
	double frameTime = timeDiff / (double)timeFrequency.QuadPart;

	ftm = frameTime;

	float multiplier = frameTime / *(float*)&fixedFrametime;

	if (engine)
	{
		engine->framerate = fps;
	}

	// Change bird speed limit
	birdMaxSpeed = multiplier * *defaultBirdMaxSpeed;

	lastTime = currTime;
}




const float LAN_FRAMETIME_GAME = 1.0f / 30.0f;

int CarDynamicsInstance::E576A0(float a2)
{
	return ((*this).*addressE576A0)(LAN_FRAMETIME_GAME);
}

int CarDynamicsInstance::E4A020(float a2)
{
	return ((*this).*addressE4A020)(LAN_FRAMETIME_GAME);
}

int CarDynamicsInstance::E4AFE0(float a2)
{
	return ((*this).*addressE4AFE0)(1.0f/165.0f);
//	return ((*this).*addressE4AFE0)(LAN_FRAMETIME_GAME);
}

void CarDynamicsInstance::E4C2F0(float a2)
{

	return ((*this).*addressE4C2F0)(LAN_FRAMETIME_GAME);
}

void CarDynamicsInstance::E490B0(float a2)
{
	return ((*this).*addressE490B0)(LAN_FRAMETIME_GAME);
}

int CarDynamicsInstance::E4F3A0(float a2)
{
	return ((*this).*addressE4F3A0)(LAN_FRAMETIME_GAME);
}

int CarDynamicsInstance::E524B0(float a2, char a3, char a4)
{
	return ((*this).*addressE524B0)(LAN_FRAMETIME_GAME, a3, a4);
}

void CarDynamicsInstance::E49830(float a2, float* a3)
{
	return ((*this).*addressE49830)(LAN_FRAMETIME_GAME, a3);
}




void __stdcall Hook_Pencil(InspectionSystem* inspection)
{	
	if (!inspection || !inspection->tb.object || !inspection->tb.object->clue || !inspection->stage) // Safe-guard
	{
		return;
	}

	if (inspection->tb.object->clue->__vftptr == notebookClueVft) // must be of type NotebookClue
	{
		if (inspection->stage->__vftptr != pairedAnimStageVft) // must be of type PairedAnimStage
		{
			return; 
		}

		if (inspection->stage->state == 2 &&
			inspection->tb.object1 == inspection->tb.object2)
		{
			inspection->stage->state = 1; // Delay next stage
		}
	}
}
