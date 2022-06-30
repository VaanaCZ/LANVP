//-----------------------------------------------------------------------------
// exports.h
// 
// Releases:
//     1.0  - Initial release
//     1.1  - "FPS Unlock" & "Aspect Correction" improvements, "Launcher Check",
//            "Skip Logo&Legals" & "FPS Lock" added, "Force Resolution" bugfix.
//     1.1a - Added "Force DX11" option, fixed a bug with force resolution.
//     1.1b - Added support for 2675, fixed dinput8.dll not found on 32-bit
//            systems, made WinAPI error messages more verbose.
// 
// Copyright (c) 2021-2022 Václav AKA Vaana
//-----------------------------------------------------------------------------

#include <windows.h>

// Types
typedef struct _DIOBJECTDATAFORMAT {
	const GUID* pguid;
	DWORD   dwOfs;
	DWORD   dwType;
	DWORD   dwFlags;
} DIOBJECTDATAFORMAT, * LPDIOBJECTDATAFORMAT;
typedef const DIOBJECTDATAFORMAT* LPCDIOBJECTDATAFORMAT;

typedef struct _DIDATAFORMAT {
	DWORD   dwSize;
	DWORD   dwObjSize;
	DWORD   dwFlags;
	DWORD   dwDataSize;
	DWORD   dwNumObjs;
	LPDIOBJECTDATAFORMAT rgodf;
} DIDATAFORMAT, * LPDIDATAFORMAT;
typedef const DIDATAFORMAT* LPCDIDATAFORMAT;

typedef HRESULT			(WINAPI *DirectInput8Create_t) (HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
typedef HRESULT			(WINAPI *DllCanUnloadNow_t)    ();
typedef HRESULT			(WINAPI *DllGetClassObject_t)  (REFCLSID, REFIID, LPVOID*);
typedef HRESULT			(WINAPI *DllRegisterServer_t)  ();
typedef HRESULT			(WINAPI *DllUnregisterServer_t)();
typedef LPCDIDATAFORMAT	(WINAPI *GetdfDIJoystick_t)    ();

// Pointers
DirectInput8Create_t  p_DirectInput8Create;
DllCanUnloadNow_t     p_DllCanUnloadNow;
DllGetClassObject_t   p_DllGetClassObject;
DllRegisterServer_t   p_DllRegisterServer;
DllUnregisterServer_t p_DllUnregisterServer;
GetdfDIJoystick_t     p_GetdfDIJoystick;

// Functions
extern "C"
{
	HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
	{
		return p_DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	}

	HRESULT WINAPI DllCanUnloadNow()
	{
		return p_DllCanUnloadNow();
	}

	HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
	{
		return p_DllGetClassObject(rclsid, riid, ppv);
	}

	HRESULT WINAPI DllRegisterServer()
	{
		return p_DllRegisterServer();
	}

	HRESULT WINAPI DllUnregisterServer()
	{
		return p_DllUnregisterServer();
	}

	LPCDIDATAFORMAT WINAPI GetdfDIJoystick()
	{
		return p_GetdfDIJoystick();
	}
}

// Import
HINSTANCE hDinput8;

#define EMPTY()
#define TOKEN(x) EMPTY() ##x

#define LOADFUNC(FUNC)                                                   \
{                                                                        \
	p_##FUNC = (TOKEN(FUNC)_t)GetProcAddress(hDinput8, #FUNC); \
	if (p_##FUNC == NULL)                                                \
	{                                                                    \
		MessageBox(NULL, TEXT(#FUNC), L"GetProcAddress failed!", MB_OK); \
	}                                                                    \
}

void LoadDinput8()
{
	// Get the path to the library
	wchar_t libPath[MAX_PATH];
	if (GetSystemDirectoryW(libPath, MAX_PATH) == 0)
	{
		wcscpy_s(libPath, L"C:\\Windows\\System32");
	}

	wcscat_s(libPath, L"\\dinput8.dll");

	// Load the actual library
	hDinput8 = LoadLibraryW(libPath);

	if (hDinput8 == NULL)
	{
		MessageBox(NULL, libPath, L"LoadLibrary failed!", MB_OK);
		return;
	}

	// Functions
	LOADFUNC(DirectInput8Create);
	LOADFUNC(DllCanUnloadNow);
	LOADFUNC(DllGetClassObject);
	LOADFUNC(DllRegisterServer);
	LOADFUNC(DllUnregisterServer);
	LOADFUNC(GetdfDIJoystick);
}

void UnloadDinput8()
{
	FreeLibrary(hDinput8);
}