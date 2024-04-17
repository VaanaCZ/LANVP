// ----------------------------------------------------------------------------
// opt_version.h
//
// Copyright (c) 2021-2024 Vaana
// ----------------------------------------------------------------------------

#pragma once

#include <windows.h>

struct Patch;

void RegisterPatch_Version();
bool ApplyPatch_Version(Patch* patch);

BOOL __stdcall Hook_VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen);
