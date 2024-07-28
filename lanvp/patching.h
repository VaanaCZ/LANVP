// ----------------------------------------------------------------------------
// patching.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAX_SIGNATURE_INDICES	10
#define MAX_SIGNATURES			30
#define MAX_PATCHES				10

#define MASK 0xFFFFFFFF
#define HERE 0xDDDDDDDD

#define SIGARG(s)	s, sizeof(s) / sizeof(s[0])

struct Patch;

typedef bool (*ApplyFunc)(Patch*);
typedef bool (*FilterFunc)(void*);

struct Signature
{
	DWORD*			signature		= nullptr;	// Byte array used as search signature
	size_t			sigLength		= 0;		// Length of array

	FilterFunc		filterFunc		= nullptr;	// Optional callback for extra filtering

	int				associatedIndex = -1;
	unsigned int	numOccurrences	= 0;		// Number of occurences
	void*			foundPtr		= nullptr;	// Pointer to the last occurence

	bool Equals(Signature& s)
	{
		return signature == s.signature && sigLength == s.sigLength && filterFunc == s.filterFunc;
	}
};

extern unsigned int numPatches;
extern Patch patches[MAX_PATCHES];
extern unsigned int numSignatures;
extern Signature signatures[MAX_SIGNATURES];

extern HANDLE process;

extern void* execMem;
extern void* execEnd;
extern void* execPtr;

int RegisterPatch(Patch patch);
int RegisterSignature(Signature signature);

struct Patch
{
	wchar_t			name[64];									// Name which will be displayed if patch fails
	int				signatureIndices[MAX_SIGNATURE_INDICES];	// Registered signatures to be searched
	int				altSignatureIndices[MAX_SIGNATURE_INDICES];	// Registered alt-signatures to be searched
	unsigned int	numSignatureIndices	= 0;					// Number of signatures
	ApplyFunc		func				= nullptr;				// Callback to be called if all signatures are found

	int AddSignature(DWORD* signature, size_t sigLength)
	{
		if (numSignatureIndices >= MAX_SIGNATURE_INDICES)
		{
			return -1;
		}

		Signature sig;
		sig.signature		= signature;
		sig.sigLength		= sigLength;
		sig.associatedIndex	= -1;

		int index = numSignatureIndices;

		signatureIndices[numSignatureIndices]		= RegisterSignature(sig);
		altSignatureIndices[numSignatureIndices]	= -1;
		numSignatureIndices++;

		return index;
	};

	int AddSignatureWithAlt(DWORD* signature, size_t sigLength, DWORD* altSignature, size_t altSigLength)
	{
		if (numSignatureIndices >= MAX_SIGNATURE_INDICES)
		{
			return -1;
		}

		Signature sig;
		sig.signature	= signature;
		sig.sigLength	= sigLength;

		Signature altSig;
		altSig.signature	= altSignature;
		altSig.sigLength	= altSigLength;

		int index = numSignatureIndices;

		signatureIndices[numSignatureIndices]		= RegisterSignature(sig);
		altSignatureIndices[numSignatureIndices]	= RegisterSignature(altSig);

		signatures[signatureIndices[numSignatureIndices]].associatedIndex = altSignatureIndices[numSignatureIndices];
		signatures[altSignatureIndices[numSignatureIndices]].associatedIndex = signatureIndices[numSignatureIndices];

		numSignatureIndices++;

		return index;
	};

	int AddSignatureWithFilter(DWORD* signature, size_t sigLength, FilterFunc filterFunc)
	{
		if (numSignatureIndices >= MAX_SIGNATURE_INDICES)
		{
			return -1;
		}

		Signature sig;
		sig.signature		= signature;
		sig.sigLength		= sigLength;
		sig.filterFunc		= filterFunc;
		sig.associatedIndex	= -1;

		int index = numSignatureIndices;

		signatureIndices[numSignatureIndices]		= RegisterSignature(sig);
		altSignatureIndices[numSignatureIndices]	= -1;
		numSignatureIndices++;

		return index;
	};

	void* GetSignature(int index, bool* isAlt = nullptr)
	{
		if (index >= numSignatureIndices)
		{
			return nullptr;
		}

		int sigIndex = -1;

		if (signatureIndices[index] != -1)
		{
			if (isAlt) *isAlt = false;
			sigIndex = signatureIndices[index];
		}
		else
		{
			if (isAlt) *isAlt = true;
			sigIndex = altSignatureIndices[index];
		}

		if (sigIndex == -1)
			return nullptr;

		return signatures[sigIndex].foundPtr;
	}

	void SetName(const wchar_t* _name)
	{
		wcscpy_s(name, sizeof(name) / sizeof(name[0]), _name);
	}
};

void HandleError(const wchar_t* title, const wchar_t* text);

void DoPatches();
bool FindSignature(Signature& sig, void* regionStart, void* regionEnd);

#pragma pack(push, 1)
struct callNear
{
	BYTE	opcode;  // E8
	DWORD	address;
};

struct callFar
{
	BYTE	opcode;  // FF
	BYTE	reg;     // 15
	DWORD	address;
};

struct jmpShort
{
	BYTE	opcode;  // EB
	BYTE	offset;
};
#pragma pack(pop)

bool MemWrite(void* ptr, void* data, size_t dataLength);
bool MemWriteNop(void* ptr, size_t nopLength);
bool MemWriteHookCall(void* ptr, void* hook);
bool MemWriteHookCallPtr(void* ptr, void** hook);
bool MemWriteHookJmp(void* ptr, void* hook);
bool MemRead(void* ptr, void* data, size_t dataLength);
bool MemReplace(void* ptr, void* data, size_t dataLength);

void* ExecCopy(void* data, size_t dataLength);
