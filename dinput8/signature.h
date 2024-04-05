
#pragma once
#include <Windows.h>

#define MAX_SIGNATURES	5
#define MAX_PATCHES	10

#define REGISTER_MASK(p, m, b, o)	\
	p.mask = m;						\
	p.maskLength = sizeof(m);		\
	p.maskingByte = b;				\
	p.maskOffset = o

typedef unsigned char byte;

struct Patch;

typedef void (*HookFunc)(Patch*);

struct Signature
{
	byte* mask = nullptr;
	size_t maskLength = 0;
	byte maskingByte = 0xFF;
	size_t maskOffset = 0; // Offset into mask which will be used for occurence pointer
	unsigned int numOccurrences = 0;
	void* lastOccurence = nullptr;
};

struct Patch
{
	TCHAR name[50];
	Signature signatures[MAX_SIGNATURES];
	unsigned int numSignatures = 0;
	HookFunc func = nullptr;

	bool RegisterSignature(Signature signature)
	{
		if (numSignatures >= MAX_PATCHES)
		{
			return false;
		}

		signatures[numSignatures] = signature;
		numSignatures++;

		return true;
	};
};

static unsigned int numPatches = 0;
static Patch patches[MAX_PATCHES] = { };

bool RegisterPatch(Patch patch);
void DoPatches();
