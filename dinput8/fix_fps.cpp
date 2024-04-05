#include "fix_fps.h"

#include "signature.h"

#define MASK 0xFF

byte framerateSignature[] =
{
	0x8B, 0xFB,
	0xC6, 0x44, 0x24, 0x48, 0x01,
	0xE8, MASK, MASK, MASK, MASK,
	0x88, 0x44, 0x24, 0x1B,
	0x84, 0xC0
};

byte rendererDestructorSignature[] =
{
	0x8B, 0x96, 0x9C, 0x00, 0x00, 0x00,
	0x83, 0xE1, 0xF8,
	0x8B, 0x01,
	0x8B, 0x40, 0x0C,
	0x52,
	0xFF, 0xD0,
	0xC7, 0x05, MASK, MASK, MASK, MASK, 0x00, 0x00, 0x00, 0x00,
	0x8B, 0x4C, 0x24, 0x10,
	0x64, 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00
};

void RegisterPatch_Framerate()
{
	Patch patch;
	
	Signature signature;
	REGISTER_MASK(signature, framerateSignature, MASK, 8);
	patch.RegisterSignature(signature);

	Signature signature2;
	REGISTER_MASK(signature2, rendererDestructorSignature, MASK, 19);
	patch.RegisterSignature(signature2);

	patch.func = DoPatch_Framerate;

	RegisterPatch(patch);
}

#pragma pack(push, 1)
class I3DEngine
{
public:
	byte padding[0xE8];
	float framerate;
};
#pragma pack(pop)


static I3DEngine** enginePtr;

void DoPatch_Framerate(Patch* patch)
{
	DWORD hookAddress = (DWORD)&DoHook_Framerate;
	hookAddress -= (DWORD)patch->signatures[0].lastOccurence;
	hookAddress -= 4;

	WriteProcessMemory(GetCurrentProcess(), patch->signatures[0].lastOccurence, &hookAddress, sizeof(hookAddress), 0);

	ReadProcessMemory(GetCurrentProcess(), patch->signatures[1].lastOccurence, &enginePtr, sizeof(enginePtr), 0);
}


static LARGE_INTEGER	lastTime, timeFrequency;
static bool			firstFrame = true;

char DoHook_Framerate(int pointer)
{
	if ((*enginePtr) != 0)
	{
		LARGE_INTEGER currTime;
		QueryPerformanceCounter(&currTime);

		if (!firstFrame)
		{
			LONGLONG quadDiff = currTime.QuadPart - lastTime.QuadPart;

			if (quadDiff > 0)
			{
				double fps = timeFrequency.QuadPart / (double)quadDiff;

				(*enginePtr)->framerate = max(fps, 30) * 2.0;
			}
		}
		else
		{
			QueryPerformanceFrequency(&timeFrequency);
			firstFrame = false;
		}

		lastTime = currTime;
	}

	return 1;
}
