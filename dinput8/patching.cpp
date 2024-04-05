#include "patching.h"

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <iostream>
#include <cassert>


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
	HANDLE process = GetCurrentProcess();
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

					if (regionPtr + signature.maskLength > regionEnd)
					{
						break; // Signature cannot be located within current region
					}

					// Signature matching
					for (size_t i = 0; i < signature.maskLength; i++)
					{
						BYTE l = *(regionPtr + i);
						BYTE r = signature.mask[i];

						if (r == signature.maskingByte)
						{
							continue;
						}

						if (l != r)
						{
							signatureValid = false;
							break;
						}
					}

					if (signatureValid)
					{
						signature.numOccurrences++;
						signature.lastOccurence = regionPtr + signature.maskOffset;
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
			bool signatureFound = (patch.signatures[j].numOccurrences == 1);

			if (!signatureFound)
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
}
