
#pragma once
#include <Windows.h>

#define MAX_SIGNATURES	10
#define MAX_PATCHES		10

#define MASK 0xFFFFFFFF
#define SKIP 0xEEEEEEEE
#define HERE 0xDDDDDDDD

#define REGISTER_MASK(p, m)					\
{											\
	Signature s;							\
	s.signature = m;						\
	s.sigLength = sizeof(m) / sizeof(m[0]);	\
	p.RegisterSignature(s);					\
}

struct Patch;

typedef bool (*ApplyFunc)(Patch*);

struct Signature
{
	DWORD*			signature		= nullptr;	// Byte array used as search signature
	size_t			sigLength		= 0;		// Length of array
	bool			optional		= false;	// Specifies whether this signature is required for a successful patch

	unsigned int	numOccurrences	= 0;		// Number of occurences
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

void DoPatches();

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
