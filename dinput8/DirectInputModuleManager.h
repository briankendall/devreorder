#pragma once

#include <string>

#include <windows.h>
#include <xinput.h>
#include "SimpleIni.h"

#include "Common.h"

class DirectInputModuleManager : NonCopyable
{
public:
	HRESULT(WINAPI* DirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
	HRESULT(WINAPI* DllCanUnloadNow)(void);
	HRESULT(WINAPI* DllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv);
	HRESULT(WINAPI* DllRegisterServer)(void);
	HRESULT(WINAPI* DllUnregisterServer)(void);

	DirectInputModuleManager()
	{
		std::wstring loadedModulePath, chainLoadFileName;
		CSimpleIniW ini;
		ini.SetAllowEmptyValues(false);
		std::wstring inipath(L"devreorder.ini");
		SI_Error err = ini.LoadFile(inipath.c_str());

		if (err < 0) {
			CheckCommonDirectory(&inipath, L"devreorder");
			err = ini.LoadFile(inipath.c_str());
		}

		if (err >= 0) {
			chainLoadFileName = ini.GetValue(L"\0", L"ChainLoadFileName", L"");
		}

		if (chainLoadFileName.size() > 0) {
			PrintLog(L"devreorder: chain loading %s", chainLoadFileName.c_str());
			FullPathFromPath(&loadedModulePath, chainLoadFileName);
			m_module = LoadLibraryW(loadedModulePath.c_str());
		} else {
			// Check to make sure we're not in the system32 directory and trying to load ourselves:
			std::wstring modulePath = thisModuleDirectory();
			std::transform(modulePath.begin(), modulePath.end(), modulePath.begin(), towlower);

			std::wstring systemDirectory = getSystemDirectoryString();
			std::transform(systemDirectory.begin(), systemDirectory.end(), systemDirectory.begin(), towlower);

			std::wstring dinput8orgPath = systemDirectory;
			StringPathAppend(&dinput8orgPath, L"dinput8org.dll");

			// If we are in system32 then the user needs to have renamed the original dll to dinput8org.dll
			// Also if that file exists we should just use that since that way we know we're not loading another
			// copy of this wrapper DLL
			if (modulePath == systemDirectory || FileExist(dinput8orgPath)) {
				m_module = LoadLibrarySystem(L"dinput8org.dll", &loadedModulePath);
			} else {
				m_module = LoadLibrarySystem(L"dinput8.dll", &loadedModulePath);
			}
		}

		if (!m_module)
		{
			HRESULT hr = GetLastError();
			std::unique_ptr<WCHAR[]> error_msg(new WCHAR[MAX_PATH]);
			swprintf_s(error_msg.get(), MAX_PATH, L"Cannot load \"%s\" error: 0x%x", loadedModulePath.c_str(), hr);
			MessageBoxW(NULL, error_msg.get(), L"Error", MB_ICONERROR);
			exit(hr);
		}
		else
		{
			PrintLog("Loaded \"%s\"", UTF16ToUTF8(loadedModulePath).c_str());
		}

		GetProcAddress("DirectInput8Create", &DirectInput8Create);
		GetProcAddress("DllCanUnloadNow", &DllCanUnloadNow);
		GetProcAddress("DllGetClassObject", &DllGetClassObject);
		GetProcAddress("DllRegisterServer", &DllRegisterServer);
		GetProcAddress("DllUnregisterServer", &DllUnregisterServer);
	}

	~DirectInputModuleManager()
	{
		if (m_module)
		{
			std::string xinput_path;
			ModulePath(&xinput_path, m_module);
			PrintLog("Unloading %s", xinput_path.c_str());
			FreeLibrary(m_module);
		}
	}

	static DirectInputModuleManager& Get()
	{
		static DirectInputModuleManager instance;
		return instance;
	}

private:
	template<typename T>
	inline void GetProcAddress(const char* funcname, T* ppfunc)
	{
		*ppfunc = reinterpret_cast<T>(::GetProcAddress(m_module, funcname));
	}

	HMODULE m_module;
};
