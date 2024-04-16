
#pragma once
#include <Windows.h>

#define MAX_SIGNATURES	10
#define MAX_PATCHES		10

#define MASK 0xFFFFFFFF
#define HERE 0xDDDDDDDD

#define REGISTER_MASK(p, m)						\
{												\
	Signature s;								\
	s.signature = m;							\
	s.sigLength = sizeof(m) / sizeof(m[0]);		\
	p.RegisterSignature(s);						\
}

#define REGISTER_MASK_ALTERNATE(p, m, a)		\
{												\
	Signature s;								\
	s.signature = m;							\
	s.sigLength = sizeof(m) / sizeof(m[0]);		\
	s.altSignature = a;							\
	s.altSigLength = sizeof(a) / sizeof(a[0]);	\
	p.RegisterSignature(s);						\
}

#define REGISTER_MASK_FILTER(p, m, f)			\
{												\
	Signature s;								\
	s.signature = m;							\
	s.sigLength = sizeof(m) / sizeof(m[0]);		\
	s.filterFunc = &f;							\
	p.RegisterSignature(s);						\
}

struct Patch;

typedef bool (*ApplyFunc)(Patch*);
typedef bool (*FilterFunc)(void*);

struct Signature
{
	DWORD*			signature		= nullptr;	// Byte array used as search signature
	size_t			sigLength		= 0;		// Length of array
	
	DWORD*			altSignature	= nullptr;	// Byte array used as search signature
	size_t			altSigLength	= 0;		// Length of array

	FilterFunc		filterFunc		= nullptr;	// Optional callback for extra filtering

	unsigned int	numOccurrences	= 0;		// Number of occurences
	bool			isAlternate		= false;	// Specifies whether altSignature was used
	void*			foundPtr		= nullptr;	// Pointer to the last occurence
};

struct Patch
{
	TCHAR			name[50];					// Name which will be displayed if patch fails
	Signature		signatures[MAX_SIGNATURES];	// Registered signatures to be searched
	unsigned int	numSignatures	= 0;		// Number of signatures
	ApplyFunc		func			= nullptr;	// Callback to be called if all signatures are found

	bool RegisterSignature(Signature signature)
	{
		if (numSignatures >= MAX_SIGNATURES)
		{
			return false;
		}

		signatures[numSignatures] = signature;
		numSignatures++;

		return true;
	};
};

extern unsigned int numPatches;
extern Patch patches[MAX_PATCHES];
extern HANDLE process;

extern void* execMem;
extern void* execEnd;
extern void* execPtr;

bool RegisterPatch(Patch patch);

void HandleError(const TCHAR* title, const TCHAR* text);

void DoPatches();
bool FindSignature(Signature& sig, bool isAlternate, void* regionStart, void* regionEnd, BYTE* regionPtr);

#pragma pack(push, 1)
struct call
{
	BYTE	opcode;  // E8
	DWORD	address;
};

struct callPtr
{
	BYTE	opcode;  // FF
	BYTE	reg;     // 15
	DWORD	address;
};

struct jmp
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
