
#define CINTERFACE
#include "stdafx.h"
#include <dinput.h>
#include <list>

#include "dinput8.h"
#include "Common.h"
#include "Logger.h"
#include "Utils.h"
#include "DirectInputModuleManager.h"
#include "SimpleIni.h"

using namespace std;

HRESULT(STDMETHODCALLTYPE *TrueEnumDevicesA) (LPDIRECTINPUT8A This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueEnumDevicesW) (LPDIRECTINPUT8W This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *EnumDevicesA) (LPDIRECTINPUT8A This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *EnumDevicesW) (LPDIRECTINPUT8W This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) = nullptr;

vector<wstring> loadAllKeysFromSectionOfIni(const wstring &section)
{
	vector<wstring> result;
	CSimpleIniW ini;
	ini.SetAllowEmptyValues(true);
	wstring inipath(L"devreorder.ini");
	SI_Error err = ini.LoadFile(inipath.c_str());

	if (err < 0) {
		CheckCommonDirectory(&inipath, L"devreorder");
		err = ini.LoadFile(inipath.c_str());

		if (err < 0) {
			PrintLog("devreorder error: devreorder.ini file found");
			return result;
		} else {
			PrintLog("devreorder: using system-wide devreorder.ini");
		}
	} else {
		PrintLog("devreorder: using program-specific devreorder.ini");
	}

	CSimpleIniW::TNamesDepend keys;
	ini.GetAllKeys(section.c_str(), keys);
	keys.sort(CSimpleIniW::Entry::LoadOrder());

	for (CSimpleIniW::TNamesDepend::iterator it = keys.begin(); it != keys.end(); ++it) {
		result.push_back(it->pItem);
	}

	return result;
}

vector<wstring> & sortedControllersW()
{
	static vector<wstring> result;
	static bool needToInitialize = true;

	if (needToInitialize) {
		needToInitialize = false;
		result = loadAllKeysFromSectionOfIni(L"order");
	}

	return result;
}

vector<string> & sortedControllersA()
{
	static vector<string> result;
	static bool needToInitialize = true;

	if (needToInitialize) {
		vector<wstring> &wideList = sortedControllersW();

		for (unsigned int i = 0; i < wideList.size(); ++i) {
			result.push_back(UTF16ToUTF8(wideList[i]));
		}

		needToInitialize = false;
	}

	return result;
}

vector<wstring> & hiddenControllersW()
{
	static vector<wstring> result;
	static bool needToInitialize = true;

	if (needToInitialize) {
		result = loadAllKeysFromSectionOfIni(L"hidden");
		needToInitialize = false;
	}

	return result;
}

vector<string> & hiddenControllersA()
{
	static vector<string> result;
	static bool needToInitialize = true;

	if (needToInitialize) {
		vector<wstring> &wideList = hiddenControllersW();

		for(unsigned int i = 0; i < wideList.size(); ++i) {
			result.push_back(UTF16ToUTF8(wideList[i]));
		}

		needToInitialize = false;
	}

	return result;
}

template <class T>
struct DeviceEnumData {
	list<T> nonsorted;
	vector<list<T> > sorted;
};

string trim(const string &str)
{
	size_t first = str.find_first_not_of(' ');

	// string is empty or nothing but whitespace
	if (string::npos == first) {
		return string();
	}

	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

wstring trim(const wstring &str)
{
	size_t first = str.find_first_not_of(' ');

	// string is empty or nothing but whitespace
	if (wstring::npos == first) {
		return wstring();
	}

	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

bool stringsAreEqual(const string &a, const char *b)
{
	return strcmp((char *)trim(a).c_str(), (char *)trim(string(b)).c_str()) == 0;
}

bool stringsAreEqual(const wstring &a, const WCHAR *b)
{
	return lstrcmpW((WCHAR *)trim(a).c_str(), (WCHAR *)trim(wstring(b)).c_str()) == 0;
}

BOOL CALLBACK enumCallbackA(LPCDIDEVICEINSTANCEA deviceInstance, LPVOID userData)
{
	DeviceEnumData<DIDEVICEINSTANCEA> *enumData = (DeviceEnumData<DIDEVICEINSTANCEA> *)userData;
	vector<string> &order = sortedControllersA();
	vector<string> &hidden = hiddenControllersA();

	for (unsigned int i = 0; i < hidden.size(); ++i) {
		if (stringsAreEqual(hidden[i], deviceInstance->tszProductName)) {
			PrintLog("devreorder: product \"%s\" is hidden", deviceInstance->tszProductName);
			return DIENUM_CONTINUE;
		}
	}

	for (unsigned int i = 0; i < order.size(); ++i) {
		if (stringsAreEqual(order[i], deviceInstance->tszProductName)) {
			PrintLog("devreorder: product \"%s\" is sorted up", deviceInstance->tszProductName);
			enumData->sorted[i].push_back(*deviceInstance);
			return DIENUM_CONTINUE;
		}
	}

	PrintLog("devreorder: product \"%s\" is not sorted differently", deviceInstance->tszProductName);
	enumData->nonsorted.push_back(*deviceInstance);
	return DIENUM_CONTINUE;
}

BOOL CALLBACK enumCallbackW(LPCDIDEVICEINSTANCEW deviceInstance, LPVOID userData)
{
	DeviceEnumData<DIDEVICEINSTANCEW> *enumData = (DeviceEnumData<DIDEVICEINSTANCEW> *)userData;
	vector<wstring> &order = sortedControllersW();
	vector<wstring> &hidden = hiddenControllersW();

	for (unsigned int i = 0; i < hidden.size(); ++i) {
		if (stringsAreEqual(hidden[i], deviceInstance->tszProductName)) {
			PrintLog(L"devreorder: product \"%s\" is hidden", deviceInstance->tszProductName);
			return DIENUM_CONTINUE;
		}
	}

	for (unsigned int i = 0; i < order.size(); ++i) {
		if (stringsAreEqual(order[i], deviceInstance->tszProductName)) {
			PrintLog(L"devreorder: product \"%s\" is sorted up", deviceInstance->tszProductName);
			enumData->sorted[i].push_back(*deviceInstance);
			return DIENUM_CONTINUE;
		}
	}

	PrintLog(L"devreorder: product \"%s\" is not sorted differently", deviceInstance->tszProductName);
	enumData->nonsorted.push_back(*deviceInstance);
	return DIENUM_CONTINUE;
}

HRESULT STDMETHODCALLTYPE HookEnumDevicesA(LPDIRECTINPUT8A This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	vector<string> &order = sortedControllersA();
	DeviceEnumData<DIDEVICEINSTANCEA> enumData;

	enumData.sorted.resize(order.size());

	PrintLog("devreorder: determining new sorting order for devices");
	HRESULT result = TrueEnumDevicesA(This, dwDevType, enumCallbackA, (LPVOID)&enumData, dwFlags);

	if (result != DI_OK) {
		return result;
	}

	for(unsigned int i = 0; i < enumData.sorted.size(); ++i) {
		for (list<DIDEVICEINSTANCEA>::iterator it = enumData.sorted[i].begin(); it != enumData.sorted[i].end(); ++it) {
			if (lpCallback(&(*it), pvRef) != DIENUM_CONTINUE) {
				return result;
			}
		}
	}

	for (list<DIDEVICEINSTANCEA>::iterator it = enumData.nonsorted.begin(); it != enumData.nonsorted.end(); ++it) {
		if (lpCallback(&(*it), pvRef) != DIENUM_CONTINUE) {
			return result;
		}
	}

	return result;
}

HRESULT STDMETHODCALLTYPE HookEnumDevicesW(LPDIRECTINPUT8W This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	vector<wstring> &order = sortedControllersW();
	DeviceEnumData<DIDEVICEINSTANCEW> enumData;

	enumData.sorted.resize(order.size());

	PrintLog("devreorder: determining new sorting order for devices");
	HRESULT result = TrueEnumDevicesW(This, dwDevType, enumCallbackW, (LPVOID)&enumData, dwFlags);

	if (result != DI_OK) {
		return result;
	}

	for (unsigned int i = 0; i < enumData.sorted.size(); ++i) {
		for (list<DIDEVICEINSTANCEW>::iterator it = enumData.sorted[i].begin(); it != enumData.sorted[i].end(); ++it) {
			if (lpCallback(&(*it), pvRef) != DIENUM_CONTINUE) {
				return result;
			}
		}
	}

	for (list<DIDEVICEINSTANCEW>::iterator it = enumData.nonsorted.begin(); it != enumData.nonsorted.end(); ++it) {
		if (lpCallback(&(*it), pvRef) != DIENUM_CONTINUE) {
			return result;
		}
	}

	return result;
}

void CreateHooks(REFIID riidltf, LPVOID *realDI)
{
	if (IsEqualIID(riidltf, IID_IDirectInput8A))
	{
		LPDIRECTINPUT8A pDIA = static_cast<LPDIRECTINPUT8A>(*realDI);

		if (pDIA)
		{
			PrintLog("devreorder: using ANSI interface");
			if (pDIA->lpVtbl->EnumDevices)
			{
				EnumDevicesA = pDIA->lpVtbl->EnumDevices;
				IH_CreateHook(EnumDevicesA, HookEnumDevicesA, &TrueEnumDevicesA);
				IH_EnableHook(EnumDevicesA);
			}
		}
	}
	else if (IsEqualIID(riidltf, IID_IDirectInput8W))
	{
		LPDIRECTINPUT8W pDIW = static_cast<LPDIRECTINPUT8W>(*realDI);

		if (pDIW)
		{
			PrintLog("devreorder: using UNICODE interface");
			if (pDIW->lpVtbl->EnumDevices)
			{
				EnumDevicesW = pDIW->lpVtbl->EnumDevices;
				IH_CreateHook(EnumDevicesW, HookEnumDevicesW, &TrueEnumDevicesW);
				IH_EnableHook(EnumDevicesW);
			}
		}
	}
}

extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
	PrintLog("devreorder: Calling hooked DirectInput8Create");
	HRESULT hr = DirectInputModuleManager::Get().DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	if (hr != DI_OK) return hr;

	CreateHooks(riidltf, ppvOut);

	return hr;
}

extern "C" HRESULT WINAPI DllCanUnloadNow(void)
{
	return DirectInputModuleManager::Get().DllCanUnloadNow();
}

extern "C" HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv)
{
	PrintLog("devreorder: Calling hooked DllGetClassObject");
	IClassFactory *cf;
	HRESULT hr = DirectInputModuleManager::Get().DllGetClassObject(rclsid, riid, (void**)&cf);

	if (hr != DI_OK) {
		return hr;
	}

	*ppv = cf;

	IDirectInput8 *realDI;
	hr = cf->lpVtbl->CreateInstance(cf, NULL, IID_IDirectInput8, (void**)&realDI);

	if (hr != DI_OK) {
		return hr;
	}

	CreateHooks(IID_IDirectInput8, (LPVOID *)&realDI);

	return hr;
}

extern "C" HRESULT WINAPI DllRegisterServer(void)
{
	return DirectInputModuleManager::Get().DllRegisterServer();
}

extern "C" HRESULT WINAPI DllUnregisterServer(void)
{
	return DirectInputModuleManager::Get().DllUnregisterServer();
}
