// ----------------------------------------------------------------------------
// patching.cpp
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#include "patching.h"

#include <psapi.h>
#include <cassert>
#include <wchar.h>

#define NO_CHECK_DUPLICATES // ENABLE FOR FINAL RELEASE

unsigned int numPatches					= 0;
Patch patches[MAX_PATCHES]				= { };
unsigned int numSignatures				= 0;
Signature signatures[MAX_SIGNATURES]	= { };

HANDLE process = NULL;

void* execMem = nullptr;
void* execEnd = nullptr;
void* execPtr = nullptr;

int RegisterPatch(Patch patch)
{
	unsigned int index = -1;

	if (numPatches >= MAX_PATCHES)
	{
		return index;
	}

	index = numPatches;

	patches[numPatches] = patch;
	numPatches++;

	return index;
}

int RegisterSignature(Signature signature)
{
	unsigned int index = -1;

	// Check if signature is already registered
	for (size_t i = 0; i < numSignatures; i++)
	{
		if (signatures[i].Equals(signature))
		{
			index = i;
			break;
		}
	}

	if (index == -1)
	{
		if (numSignatures >= MAX_SIGNATURES)
		{
			return -1;
		}

		index = numSignatures;

		size_t newIndex = 0;
		size_t newLength = signature.rawLength;

		for (size_t i = 0; i < signature.rawLength; i++)
		{
			if (signature.rawSignature[i] == HERE)
			{
				newLength--;
				signature.sigOffset = i;
			}
		}

		signature.sigLength = newLength;

		if (newLength % SIG_ALIGN)
		{
			newLength = ((newLength / SIG_ALIGN) + 1) * SIG_ALIGN; // Align to 4 bytes because we compare DWORDs instead of BYTEs
		}

		signature.sigPattern	= new BYTE[newLength];
		signature.sigMask		= new BYTE[newLength];

		memset(signature.sigPattern, 0, newLength);
		memset(signature.sigMask, 0, newLength);

		for (size_t i = 0; i < signature.rawLength; i++)
		{
			if (signature.rawSignature[i] == HERE)
			{
				continue;
			}

			if (signature.rawSignature[i] != MASK)
			{
				signature.sigPattern[newIndex]	= signature.rawSignature[i];
				signature.sigMask[newIndex]		= 0xFF;
			}

			newIndex++;
		}

		signature.sigLength = newLength;

		signatures[numSignatures] = signature;
		numSignatures++;
	}

	return index;
}

bool IsUltimateASILoader()
{
	const size_t stackSize = 1024;
	void* stack[stackSize];
	WORD numFrames = CaptureStackBackTrace(0, stackSize, stack, NULL);

	for (size_t i = 0; i < numFrames; i++)
	{
		void* addr = stack[i];
		HMODULE module = NULL;

		if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)addr, &module))
		{
			if (GetProcAddress(module, "IsUltimateASILoader"))
				return true;
		}
	}

	return false;
}

void HandleError(const wchar_t* title, const wchar_t* text)
{
	const size_t messageLength = 1024;
	wchar_t message[messageLength];

	wchar_t* errorMessage = nullptr;
	DWORD error = GetLastError();

	if (error > 0)
	{
		size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (wchar_t*)&errorMessage, 0, NULL);
	}

	swprintf_s(message, messageLength, L"%s %s", text, errorMessage);

	MessageBoxW(NULL, message, title, MB_OK);
}

const wchar_t doPatchesErrorTitle[]				= L"[V-PATCH] Error while initializing patch!";
const wchar_t memWriteErrorTitle[]				= L"[V-PATCH] Error while writing memory!";
const wchar_t memWriteNopErrorTitle[]			= L"[V-PATCH] Error while writing NOP to memory!";
const wchar_t memReadErrorTitle[]				= L"[V-PATCH] Error while reading memory!";
const wchar_t memReplaceErrorTitle[]			= L"[V-PATCH] Error while replacing memory!";

const wchar_t virtualQueryErrorText[]			= L"Failed to query virtual memory.";
const wchar_t virtualProtectErrorText[]			= L"Failed to change memory protection.";
const wchar_t virtualProtectRestoreErrorText[]	= L"Failed to restore memory protection.";
const wchar_t flushInstructionCacheErrorText[]	= L"Failed to flush instruction cache.";

void DoPatches()
{
	int remainingSignatureIndices[MAX_SIGNATURES];
	unsigned int numRemainingSignatures = numSignatures;

	for (size_t i = 0; i < numSignatures; i++)
		remainingSignatureIndices[i] = i;

	//
	// Memory search
	//

	// Get info about the main module
	process = GetCurrentProcess();
	HMODULE module = GetModuleHandle(NULL);

	if (!module)
	{
		HandleError(doPatchesErrorTitle, L"Failed to get main module handle.");
		return;
	}

	MODULEINFO moduleInfo;
	if (!GetModuleInformation(process, module, &moduleInfo, sizeof(moduleInfo)))
	{
		HandleError(doPatchesErrorTitle, L"Failed to get module information.");
		return;
	}

	void* start = moduleInfo.lpBaseOfDll;
	void* end = (BYTE*)start + moduleInfo.SizeOfImage;

	// Loop through memory sections
	void* ptr = start;

	while (ptr < end)
	{
		MEMORY_BASIC_INFORMATION memoryInfo;
		if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
		{
			HandleError(doPatchesErrorTitle, virtualQueryErrorText);
			return;
		}

		// Ignore non-execute pages (small speedup)
		if (!(memoryInfo.Protect & PAGE_EXECUTE) &&
			!(memoryInfo.Protect & PAGE_EXECUTE_READ) &&
			!(memoryInfo.Protect & PAGE_EXECUTE_READWRITE) &&
			!(memoryInfo.Protect & PAGE_EXECUTE_WRITECOPY))
		{
			ptr = (BYTE*)ptr + memoryInfo.RegionSize;
			continue;
		}

		// Unlock the area for reads
		DWORD oldProtection;
		bool wasProtected = false;

		if (!(memoryInfo.Protect & PAGE_EXECUTE_READ))
		{
			wasProtected = true;

			if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READ, &oldProtection))
			{
				HandleError(doPatchesErrorTitle, virtualProtectErrorText);
				return;
			}
		}

		// Try to match signatures
		void* regionStart	= memoryInfo.BaseAddress;
		void* regionEnd		= (BYTE*)regionStart + memoryInfo.RegionSize;

		for (size_t i = 0; i < numRemainingSignatures; i++)
		{
			Signature& signature = signatures[remainingSignatureIndices[i]];

			if (FindSignature(signature, regionStart, regionEnd))
			{
#ifdef NO_CHECK_DUPLICATES
				numRemainingSignatures--;
				remainingSignatureIndices[i] = remainingSignatureIndices[numRemainingSignatures];
				i--;

				if (signature.associatedIndex != -1)
				{
					for (size_t j = 0; j < numRemainingSignatures; j++)
					{
						if (remainingSignatureIndices[j] == signature.associatedIndex)
						{
							numRemainingSignatures--;
							remainingSignatureIndices[j] = remainingSignatureIndices[numRemainingSignatures];
							if (j <= i)
								i = j - 1;

							break;
						}
					}
				}
#endif
			}
		}

		// Restore protection
		if (wasProtected)
		{
			if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
			{
				HandleError(doPatchesErrorTitle, virtualProtectRestoreErrorText);
				return;
			}
		}

		ptr = (BYTE*)ptr + memoryInfo.RegionSize;

#ifdef NO_CHECK_DUPLICATES
		if (numRemainingSignatures == 0)
			break;
#endif
	}

	//
	// Prepare execute memory for patches
	//

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	execMem = VirtualAlloc(nullptr, systemInfo.dwPageSize, MEM_COMMIT, PAGE_READWRITE);

	if (!execMem)
	{
		HandleError(doPatchesErrorTitle, L"Could not allocate instruction buffer.");
		return;
	}

	execPtr = execMem;
	execEnd = (BYTE*)execMem + systemInfo.dwPageSize;

	//
	// Result
	//

	bool allPatchesApplied = true;

	const size_t messageLength = 1024;
	wchar_t message[messageLength] = L"Failed to apply the following patches:\n";

	for (size_t i = 0; i < numPatches; i++)
	{
		Patch& patch = patches[i];

		bool allSignaturesFound = true;
		enum { ERROR_NOTFOUND, ERROR_TOO_MANY_OCCURRENCES, PATCH_CANNOT_APPLY };
		const wchar_t* errorMsgs[] = { L" (No signatures found)", L" (Too many signatures found)", L" (Could not apply)" };
		int error = 0;

		for (size_t j = 0; j < patch.numSignatureIndices; j++)
		{
			int signatureIndex		= patch.signatureIndices[j];
			int altSignatureIndex	= patch.altSignatureIndices[j];

			// Only one of the signatures must be found (if both are found then its invalid)
			unsigned int totalNumOccurences = signatures[signatureIndex].numOccurrences;

			if (altSignatureIndex != -1)
				totalNumOccurences += signatures[altSignatureIndex].numOccurrences;

			if (totalNumOccurences != 1)
			{
				error = (totalNumOccurences > 1);
				allSignaturesFound = false;
				break;
			}

			// Discard one of the indices to signal which one was found			
			if (altSignatureIndex != -1 && signatures[altSignatureIndex].numOccurrences == 1)
			{
				patch.signatureIndices[j] = -1;
			}
			else
			{
				patch.altSignatureIndices[j] = -1;
			}
		}

		if (allSignaturesFound)
		{
			// Success! Patch can be applied!
			assert(patch.func);
			allSignaturesFound = patch.func(&patch);

			error = (!allSignaturesFound) * PATCH_CANNOT_APPLY;
		}

		if (!allSignaturesFound)
		{
			size_t length = wcslen(message);
			wcscpy_s(&message[length], messageLength - length, L"\n    ");
			length = wcslen(message);
			wcscpy_s(&message[length], messageLength - length, patch.name);
			length = wcslen(message);
			wcscpy_s(&message[length], messageLength - length, errorMsgs[error]);
		}

		allPatchesApplied &= allSignaturesFound;
	}

	if (!allPatchesApplied)
	{
		MessageBoxW(NULL, message, doPatchesErrorTitle, MB_OK);
	}

	//
	// Finish up
	//
	DWORD oldProtect;
	if (!VirtualProtect(execMem, systemInfo.dwPageSize, PAGE_EXECUTE_READ, &oldProtect))
	{
		HandleError(doPatchesErrorTitle, L"Failed to change memory protection on instruction buffer.");
		return;
	}

	if (!FlushInstructionCache(process, execMem, systemInfo.dwPageSize))
	{
		HandleError(doPatchesErrorTitle, L"Failed to flush instruction cache on instruction buffer.");
		return;
	}
}

// No matter how much I try to optimize this shitty routine it always becomes slower
// Compiler doing weird things here
bool FindSignature(Signature& sig, void* regionStart, void* _regionEnd)
{
	DWORD* sigPattern = (DWORD*)sig.sigPattern;
	DWORD* sigMask = (DWORD*)sig.sigMask;
	size_t sigLength = sig.sigLength / sizeof(DWORD);

	BYTE* regionPtr = (BYTE*)regionStart;
	BYTE* regionEnd = (BYTE*)_regionEnd - sig.sigLength;

	while (regionPtr < regionEnd)
	{
		DWORD* regionPtrLong = (DWORD*)regionPtr;

		bool signatureValid = true;

		// Signature matching
		for (size_t i = 0; i < sigLength; i++)
		{
			DWORD l = regionPtrLong[i];
			DWORD r = sigPattern[i];
			DWORD m = sigMask[i];

			if ((l & m) != (r & m)) // FIXME: For some reason this produces faster assembly than just (l & m) != r
			{
				signatureValid = false;
				break;
			}
		}

		void* foundPtr = regionPtr + sig.sigOffset;

		if (signatureValid && (!sig.filterFunc || sig.filterFunc(foundPtr)))
		{
			sig.numOccurrences++;
			sig.foundPtr = foundPtr;

#ifdef NO_CHECK_DUPLICATES
			break;
#endif
		}

		regionPtr++;
	}

	return (sig.numOccurrences != 0);
}

void CleanUp()
{
	for (size_t i = 0; i < numSignatures; i++)
	{
		Signature& signature = signatures[i];
		
		if (signature.sigPattern)	delete[] signature.sigPattern;
		if (signature.sigMask)		delete[] signature.sigMask;
	}
}

bool MemWrite(void* ptr, void* data, size_t dataLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		HandleError(memWriteErrorTitle, virtualQueryErrorText);
		return false;
	}

	// Unlock the area for writes
	DWORD oldProtection;
	bool wasProtected = false;

	if (!(memoryInfo.Protect & PAGE_EXECUTE_READWRITE))
	{
		wasProtected = true;

		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			HandleError(memWriteErrorTitle, virtualProtectErrorText);
			return false;
		}
	}

	// Perform write
	memcpy(ptr, data, dataLength);

	// Restore protection
	if (wasProtected)
	{
		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
		{
			HandleError(memWriteErrorTitle, virtualProtectRestoreErrorText);
			return false;
		}

		if (!FlushInstructionCache(process, ptr, dataLength))
		{
			HandleError(memWriteErrorTitle, flushInstructionCacheErrorText);
			return false;
		}
	}

	return true;
}

bool MemWriteNop(void* ptr, size_t nopLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		HandleError(memWriteNopErrorTitle, virtualQueryErrorText);
		return false;
	}

	// Unlock the area for writes
	DWORD oldProtection;
	bool wasProtected = false;

	if (!(memoryInfo.Protect & PAGE_EXECUTE_READWRITE))
	{
		wasProtected = true;

		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			HandleError(memWriteNopErrorTitle, virtualProtectErrorText);
			return false;
		}
	}

	// Perform write
	memset(ptr, 0x90, nopLength);

	// Restore protection
	if (wasProtected)
	{
		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
		{
			HandleError(memWriteNopErrorTitle, virtualProtectRestoreErrorText);
			return false;
		}

		if (!FlushInstructionCache(process, ptr, nopLength))
		{
			HandleError(memWriteNopErrorTitle, flushInstructionCacheErrorText);
			return false;
		}
	}

	return true;
}

bool MemWriteHookCall(void* ptr, void* hook)
{
	callNear c = { 0xE8, (DWORD)hook - (DWORD)ptr - sizeof(callNear)};
	return MemWrite(ptr, &c, sizeof(c));
}

bool MemWriteHookCallPtr(void* ptr, void** hook)
{
	callFar c = { 0xFF, 0x15, (DWORD)hook };
	return MemWrite(ptr, &c, sizeof(c));
}

bool MemWriteHookJmp(void* ptr, void* hook)
{
	callNear c = { 0xE9, (DWORD)hook - (DWORD)ptr - sizeof(callNear) };
	return MemWrite(ptr, &c, sizeof(c));
}

bool MemRead(void* ptr, void* data, size_t dataLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		HandleError(memReadErrorTitle, virtualQueryErrorText);
		return false;
	}

	// Unlock the area for reads
	DWORD oldProtection;
	bool wasProtected = false;

	if (!(memoryInfo.Protect & PAGE_EXECUTE_READ))
	{
		wasProtected = true;

		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READ, &oldProtection))
		{
			HandleError(memReadErrorTitle, virtualProtectErrorText);
			return false;
		}
	}

	// Perform read
	memcpy(data, ptr, dataLength);

	// Restore protection
	if (wasProtected)
	{
		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
		{
			HandleError(memReadErrorTitle, virtualProtectRestoreErrorText);
			return false;
		}
	}

	return true;
}

bool MemReplace(void* ptr, void* data, size_t dataLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		HandleError(memReplaceErrorTitle, virtualQueryErrorText);
		return false;
	}

	// Unlock the area for writes
	DWORD oldProtection;
	bool wasProtected = false;

	if (!(memoryInfo.Protect & PAGE_EXECUTE_READWRITE))
	{
		wasProtected = true;

		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			HandleError(memReplaceErrorTitle, virtualProtectErrorText);
			return false;
		}
	}

	// Perform write
	BYTE* cPtr = (BYTE*)ptr;
	BYTE* cData = (BYTE*)data;

	BYTE tmp;
	for (size_t i = 0; i < dataLength; i++)
	{
		tmp = cPtr[i];
		cPtr[i] = cData[i];
		cData[i] = tmp;
	}

	// Restore protection
	if (wasProtected)
	{
		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
		{
			HandleError(memReplaceErrorTitle, virtualProtectRestoreErrorText);
			return false;
		}

		if (!FlushInstructionCache(process, ptr, dataLength))
		{
			HandleError(memReplaceErrorTitle, flushInstructionCacheErrorText);
			return false;
		}
	}

	return true;
}

void* ExecCopy(void* data, size_t dataLength)
{
	if ((BYTE*)execPtr + dataLength > execEnd)
	{
		assert(false);
		return nullptr;
	}

	memcpy(execPtr, data, dataLength);

	void* result = execPtr;
	execPtr = (BYTE*)execPtr + dataLength;

	return result;
}
