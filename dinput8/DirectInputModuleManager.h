#pragma once

#include <string>

#include <windows.h>
#include <xinput.h>

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
		IniFile ini;
		std::string inipath("devreorder.ini");
		if (!ini.Load(inipath))
		{
			CheckCommonDirectory(&inipath, "devreorder");
			ini.Load(inipath);
		}
		std::string loaded_module_path;
		std::string chainload_filename;

		if (ini.Get("DINPUT8", "ChainLoadFileName", &chainload_filename))
		{
			FullPathFromPath(&loaded_module_path, chainload_filename);
			m_module = LoadLibraryA(loaded_module_path.c_str());
		}
		else
		{
			// Check to make sure we're not in the system32 directory and trying to load ourselves:
			std::wstring modulePath = thisModuleDirectory();
			std::transform(modulePath.begin(), modulePath.end(), modulePath.begin(), towlower);

			std::wstring systemDirectory = getSystemDirectoryString();
			std::transform(systemDirectory.begin(), systemDirectory.end(), systemDirectory.begin(), towlower);

			OutputDebugStringW(modulePath.c_str());
			OutputDebugStringW(systemDirectory.c_str());

			if (modulePath == systemDirectory) {
				// If we are in system32 then the user needs to have renamed  the original dll to dinput8org.dll:
				m_module = LoadLibrarySystem("dinput8org.dll", &loaded_module_path);
			} else {
				m_module = LoadLibrarySystem("dinput8.dll", &loaded_module_path);
			}
		}

		if (!m_module)
		{
			HRESULT hr = GetLastError();
			std::unique_ptr<char[]> error_msg(new char[MAX_PATH]);
			sprintf_s(error_msg.get(), MAX_PATH, "Cannot load \"%s\" error: 0x%x", loaded_module_path.c_str(), hr);
			PrintLog(error_msg.get());
			MessageBoxA(NULL, error_msg.get(), "Error", MB_ICONERROR);
			exit(hr);
		}
		else
		{
			PrintLog("Loaded \"%s\"", loaded_module_path.c_str());
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
