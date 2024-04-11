#include "patching.h"

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <iostream>
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
		assert(false);
	}

	MODULEINFO moduleInfo;
	if (!GetModuleInformation(process, module, &moduleInfo, sizeof(moduleInfo)))
	{
		assert(false);
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
			assert(false);
		}

		// Unlock the area for reads
		DWORD oldProtection;
		bool wasProtected = false;

		if (!(memoryInfo.Protect & PAGE_EXECUTE_READ))
		{
			wasProtected = true;

			if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READ, &oldProtection))
			{
				assert(false);
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

					bool signatureValid = true;

					if (regionPtr + signature.sigLength > regionEnd)
					{
						break; // Signature cannot be located within current region
					}

					// Signature matching
					for (size_t i = 0; i < signature.sigLength; i++)
					{
						if (signature.signature[i] == MASK)
						{
							continue;
						}

						assert(signature.signature[i] <= 0xFF);

						BYTE l = *(regionPtr + i);
						BYTE r = signature.signature[i];

						if (l != r)
						{
							signatureValid = false;
							break;
						}
					}

					if (signatureValid)
					{
						signature.numOccurrences++;
						signature.foundPtr = regionPtr + signature.sigOffset;
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
				assert(false);
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
	execPtr = execMem;
	execEnd = (byte*)execMem + systemInfo.dwPageSize;

	assert(execMem);

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
			if (signature.numOccurrences != 1 && !signature.optional)
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
	VirtualProtect(execMem, systemInfo.dwPageSize, PAGE_EXECUTE_READ, &oldProtect);

}

bool MemWrite(void* ptr, void* data, size_t dataLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		assert(false);
	}

	// Unlock the area for writes
	DWORD oldProtection;
	bool wasProtected = false;

	if (!(memoryInfo.Protect & PAGE_EXECUTE_READWRITE))
	{
		wasProtected = true;

		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			assert(false);
		}
	}

	// Perform write
	memcpy(ptr, data, dataLength);

	// Restore protection
	if (wasProtected)
	{
		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
		{
			assert(false);
		}

		if (!FlushInstructionCache(process, ptr, dataLength))
		{
			assert(false);
		}
	}

	return true;
}

bool MemWriteNop(void* ptr, size_t nopLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		assert(false);
	}

	// Unlock the area for writes
	DWORD oldProtection;
	bool wasProtected = false;

	if (!(memoryInfo.Protect & PAGE_EXECUTE_READWRITE))
	{
		wasProtected = true;

		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			assert(false);
		}
	}

	// Perform write
	memset(ptr, 0x90, nopLength);

	// Restore protection
	if (wasProtected)
	{
		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
		{
			assert(false);
		}

		if (!FlushInstructionCache(process, ptr, nopLength))
		{
			assert(false);
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
		assert(false);
	}

	// Unlock the area for reads
	DWORD oldProtection;
	bool wasProtected = false;

	if (!(memoryInfo.Protect & PAGE_EXECUTE_READ))
	{
		wasProtected = true;

		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READ, &oldProtection))
		{
			assert(false);
		}
	}

	// Perform read
	memcpy(data, ptr, dataLength);

	// Restore protection
	if (wasProtected)
	{
		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, oldProtection, &oldProtection))
		{
			assert(false);
		}
	}

	return true;
}

bool MemReplace(void* ptr, void* data, size_t dataLength)
{
	MEMORY_BASIC_INFORMATION memoryInfo;
	if (!VirtualQueryEx(process, ptr, &memoryInfo, sizeof(memoryInfo)))
	{
		assert(false);
	}

	// Unlock the area for writes
	DWORD oldProtection;
	bool wasProtected = false;

	if (!(memoryInfo.Protect & PAGE_EXECUTE_READWRITE))
	{
		wasProtected = true;

		if (!VirtualProtectEx(process, memoryInfo.BaseAddress, memoryInfo.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtection))
		{
			assert(false);
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
			assert(false);
		}

		if (!FlushInstructionCache(process, ptr, dataLength))
		{
			assert(false);
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
