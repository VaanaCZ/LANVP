#include "patching.h"

#include <windows.h>
#include <psapi.h>

#include <cassert>

unsigned int numPatches = 0;
Patch patches[MAX_PATCHES] = { };
HANDLE process = 0;
void* execMem = nullptr;
void* execEnd = nullptr;
void* execPtr = nullptr;

bool RegisterPatch(Patch patch)
{
	if (numPatches >= MAX_PATCHES)
	{
		return false;
	}

	patches[numPatches] = patch;
	numPatches++;

	return true;
}

void HandleError(const TCHAR* title, const TCHAR* text)
{
	const size_t msgSize = 1000;
	TCHAR errorMsg[msgSize];
	TCHAR* msgPtr = errorMsg;
	TCHAR* msgEnd = errorMsg + msgSize;

	ua_tcscpy_s(msgPtr, msgEnd - msgPtr, text);
	msgPtr += ua_lstrlen(text);

	DWORD error = GetLastError();

	if (error > 0)
	{
		TCHAR* message = nullptr;

		size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(TCHAR*)&message,
			0,
			NULL);

		if (size > 0)
		{
			ua_tcscpy_s(msgPtr, msgEnd - msgPtr, message);
			msgPtr += ua_lstrlen(message);
		}
	}

	MessageBox(NULL, errorMsg, title, MB_OK);
}

#define SKIP_PROBE 32

// fixme: constants for error msg

void DoPatches()
{
	//
	// Memory search
	//

	// Get info about the main module
	process = GetCurrentProcess();
	HMODULE module = GetModuleHandle(NULL);

	if (!module)
	{
		HandleError(TEXT("Patch initialisation fail!"), TEXT("Failed to get main module handle."));
		return;
	}

	MODULEINFO moduleInfo;
	if (!GetModuleInformation(process, module, &moduleInfo, sizeof(moduleInfo)))
	{
		HandleError(TEXT("Patch initialisation fail!"), TEXT("Failed to get module information."));
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
			HandleError(TEXT("Patch initialisation fail!"), TEXT("Failed to query virtual memory."));
			return;
		}

		// Unlock the area for reads
		DWORD oldProtection;
		bool wasProtected = false;

		if (!(memoryInfo.Protect & PAGE_EXECUTE_READ))
		{
			wasProtected = true;

			if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READ, &oldProtection))
			{
				HandleError(TEXT("Patch initialisation fail!"), TEXT("Failed to change memory protection."));
				return;
			}
		}

		// Try to match signatures
		void* regionStart = memoryInfo.BaseAddress;
		void* regionEnd = (BYTE*)regionStart + memoryInfo.RegionSize;

		BYTE* regionPtr = (BYTE*)regionStart;

		while (regionPtr < regionEnd)
		{
			for (size_t i = 0; i < numPatches; i++)
			{
				Patch& patch = patches[i];

				for (size_t j = 0; j < patch.numSignatures; j++)
				{
					Signature& signature = patch.signatures[j];

					bool rescanSignature = !FindSignature(signature, false, regionStart, regionEnd, regionPtr)
						&& signature.altSignature != nullptr;

					if (rescanSignature)
					{
						FindSignature(signature, true, regionStart, regionEnd, regionPtr);
					}
				}
			}

			regionPtr++;
		}

		// Restore protection
		if (wasProtected)
		{
			if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
			{
				HandleError(TEXT("Patch initialisation fail!"), TEXT("Failed to restore memory protection."));
				return;
			}
		}

		ptr = (BYTE*)ptr + memoryInfo.RegionSize;
	}

	//
	// Prepare execute memory for patches
	//

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	execMem = VirtualAlloc(nullptr, systemInfo.dwPageSize, MEM_COMMIT, PAGE_READWRITE);

	if (!execMem)
	{
		HandleError(TEXT("Patch initialisation fail!"), TEXT("Could not allocate instruction buffer."));
		return;
	}

	execPtr = execMem;
	execEnd = (byte*)execMem + systemInfo.dwPageSize;

	//
	// Result
	//

	bool allPatchesApplied = true;

	const size_t msgSize = 1000;
	TCHAR errorMsg[msgSize];
	TCHAR* msgPtr = errorMsg;
	TCHAR* msgEnd = errorMsg + msgSize;

	for (size_t i = 0; i < numPatches; i++)
	{
		Patch& patch = patches[i];

		ua_tcscpy_s(msgPtr, msgEnd - msgPtr, TEXT("Failed to apply the following patches:"));
		msgPtr += 38;

		bool allSignaturesFound = true;
		for (size_t j = 0; j < patch.numSignatures; j++)
		{
			Signature& signature = patch.signatures[j];
			if (signature.numOccurrences != 1/* && !signature.optional*/)
			{
				allSignaturesFound = false;
				break;
			}
		}

		if (allSignaturesFound)
		{
			// Success! Patch can be applied!
			assert(patch.func);
			allSignaturesFound = patch.func(&patch);
		}

		if (!allSignaturesFound)
		{
			ua_tcscpy_s(msgPtr, msgEnd - msgPtr, TEXT("\n\t"));
			msgPtr += 2;
			ua_tcscpy_s(msgPtr, msgEnd - msgPtr, patch.name);
			msgPtr += ua_lstrlen(patch.name);
		}

		allPatchesApplied &= allSignaturesFound;
	}

	if (!allPatchesApplied)
	{
		MessageBox(NULL, errorMsg, TEXT("[ERROR]"), MB_OK);
	}

	//
	// Finish up
	//
	DWORD oldProtect;
	if (!VirtualProtect(execMem, systemInfo.dwPageSize, PAGE_EXECUTE_READ, &oldProtect))
	{
		HandleError(TEXT("Patch initialisation fail!"), TEXT("Failed to change memory protection on instruction buffer."));
		return;
	}

	if (!FlushInstructionCache(process, execMem, systemInfo.dwPageSize))
	{
		HandleError(TEXT("Patch initialisation fail!"), TEXT("Failed to flush instruction cache on instruction buffer."));
		return;
	}
}

bool FindSignature(Signature& sig, bool isAlternate, void* regionStart, void* regionEnd, BYTE* regionPtr)
{
	bool signatureValid = true;

	DWORD* signature = sig.isAlternate ? sig.altSignature : sig.signature;
	size_t sigLength = sig.isAlternate ? sig.altSigLength : sig.sigLength;

	if (regionPtr + sigLength > regionEnd)
	{
		return false; // Signature cannot be located within current region
	}

	// Signature matching
	size_t sigIndex = 0;
	size_t sigOffset = 0;

	for (size_t i = 0; i < sigLength; i++)
	{
		if (signature[i] == HERE)
		{
			sigOffset = sigIndex;
			continue;
		}

		// Skipping logic allows to skip a variable number of bytes
		/*if (signature[i] == SKIP)
		{
			// Find the next valid byte in signature
			DWORD nextValidByte = 0xFFFFFFFF;

			for (size_t j = 0; j < SKIP_PROBE; j++)
			{
				if ((i + j) < sigLength && signature[i + j] <= 0xFF)
				{
					nextValidByte = signature[i + j];
					break;
				}
			}

			if (nextValidByte == 0xFFFFFFFF)
			{
				signatureValid = false;
				break;
			}

			// Skip any amount of bytes until the correct signature byte is found
			bool skipped = false;

			BYTE r = nextValidByte;

			for (size_t j = 0; j < SKIP_PROBE; j++)
			{
				if (regionPtr + sigIndex > regionEnd)
				{
					break; // probe too far!
				}

				BYTE l = *(regionPtr + sigIndex);

				if (l == r)
				{
					skipped = true;
					break;
				}

				sigIndex++;
			}

			if (!skipped)
			{
				signatureValid = false;
				break;
			}

			continue;
		}*/

		if (signature[i] == MASK)
		{
			sigIndex++;
			continue;
		}

		assert(signature[i] <= 0xFF);

		BYTE l = *(regionPtr + sigIndex);
		BYTE r = signature[i];

		if (l != r)
		{
			signatureValid = false;
			break;
		}

		sigIndex++;
	}

	if (signatureValid)
	{
		sig.numOccurrences++;
		sig.isAlternate = isAlternate;
		sig.foundPtr = regionPtr + sigOffset;
	}

	return true;
}

bool MemWrite(void* ptr, void* data, size_t dataLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		HandleError(TEXT("Patch memory write fail!"), TEXT("Failed to query virtual memory."));
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
			HandleError(TEXT("Patch memory write fail!"), TEXT("Failed to change memory protection."));
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
			HandleError(TEXT("Patch memory write fail!"), TEXT("Failed to restore memory protection."));
			return false;
		}

		if (!FlushInstructionCache(process, ptr, dataLength))
		{
			HandleError(TEXT("Patch memory write fail!"), TEXT("Failed to flush instruction cache."));
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
		HandleError(TEXT("Patch memory nop fail!"), TEXT("Failed to query virtual memory."));
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
			HandleError(TEXT("Patch memory nop fail!"), TEXT("Failed to change memory protection."));
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
			HandleError(TEXT("Patch memory nop fail!"), TEXT("Failed to restore memory protection."));
			return false;
		}

		if (!FlushInstructionCache(process, ptr, nopLength))
		{
			HandleError(TEXT("Patch memory nop fail!"), TEXT("Failed to flush instruction cache."));
			return false;
		}
	}

	return true;
}

bool MemWriteHookCall(void* ptr, void* hook)
{
	call c = { 0xE8, (DWORD)hook - (DWORD)ptr - sizeof(call)};
	return MemWrite(ptr, &c, sizeof(c));
}

bool MemWriteHookCallPtr(void* ptr, void** hook)
{
	callPtr c = { 0xFF, 0x15, (DWORD)hook };
	return MemWrite(ptr, &c, sizeof(c));
}

bool MemWriteHookJmp(void* ptr, void* hook)
{
	call c = { 0xE9, (DWORD)hook - (DWORD)ptr - sizeof(call) };
	return MemWrite(ptr, &c, sizeof(c));
}

bool MemRead(void* ptr, void* data, size_t dataLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		HandleError(TEXT("Patch memory read fail!"), TEXT("Failed to query virtual memory."));
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
			HandleError(TEXT("Patch memory read fail!"), TEXT("Failed to change memory protection."));
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
			HandleError(TEXT("Patch memory read fail!"), TEXT("Failed to restore memory protection."));
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
		HandleError(TEXT("Patch memory replace fail!"), TEXT("Failed to query virtual memory."));
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
			HandleError(TEXT("Patch memory replace fail!"), TEXT("Failed to change memory protection."));
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
			HandleError(TEXT("Patch memory replace fail!"), TEXT("Failed to restore memory protection."));
			return false;
		}

		if (!FlushInstructionCache(process, ptr, dataLength))
		{
			HandleError(TEXT("Patch memory replace fail!"), TEXT("Failed to flush instruction cache."));
			return false;
		}
	}

	return true;
}

void* ExecCopy(void* data, size_t dataLength)
{
	if ((byte*)execPtr + dataLength > execEnd)
	{
		assert(false);
		return nullptr;
	}

	memcpy(execPtr, data, dataLength);

	void* result = execPtr;
	execPtr = (byte*)execPtr + dataLength;

	return result;
}
