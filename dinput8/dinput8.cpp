
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
	}

	if (err < 0) {
		PrintLog("Error: devreorder.ini file found");
		return result;
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

vector<string> & sortedControllers(const string &ignored)
{
	(void)ignored;
	return sortedControllersA();
}

vector<wstring> & sortedControllers(const wstring &ignored)
{
	(void)ignored;
	return sortedControllersW();
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

vector<string> & hiddenControllers(const string &ignored)
{
	(void)ignored;
	return hiddenControllersA();
}

vector<wstring> & hiddenControllers(const wstring &ignored)
{
	(void)ignored;
	return hiddenControllersW();
}

template <class T>
struct DeviceEnumData {
	list<T> nonsorted;
	vector<list<T> > sorted;
};

char *trim(char *str)
{
	char *end;

	// Trim leading space
	while (isspace((char)*str)) str++;

	if (*str == 0) return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && isspace((char)*end)) end--;

	// Write new null terminator
	*(end + 1) = 0;

	return str;
}

WCHAR *trim(WCHAR *str)
{
	WCHAR *end;

	// Trim leading space
	while (isspace((WCHAR)*str)) str++;

	if (*str == 0) return str;

	// Trim trailing space
	end = str + lstrlenW(str) - 1;
	while (end > str && isspace((WCHAR)*end)) end--;

	// Write new null terminator
	*(end + 1) = 0;

	return str;
}


bool stringsAreEqual(const string &a, const char *b)
{
	return strcmp(trim((char *)a.c_str()), trim((char *)b)) == 0;
}

bool stringsAreEqual(const wstring &a, const WCHAR *b)
{
	return lstrcmpW(trim((WCHAR *)a.c_str()), trim((WCHAR *)b)) == 0;
}

template <class DeviceType, class StringType>
bool enumCallback(const DeviceType *deviceInstance, LPVOID userData)
{
	DeviceEnumData<DeviceType> *enumData = (DeviceEnumData<DeviceType> *)userData;
	vector<StringType> &order = sortedControllers(StringType());
	vector<StringType> &hidden = hiddenControllers(StringType());

	for (unsigned int i = 0; i < hidden.size(); ++i) {
		if (stringsAreEqual(hidden[i], deviceInstance->tszProductName)) {
			return DIENUM_CONTINUE;
		}
	}

	for (unsigned int i = 0; i < order.size(); ++i) {
		if (stringsAreEqual(order[i], deviceInstance->tszProductName)) {
			enumData->sorted[i].push_back(*deviceInstance);
			return DIENUM_CONTINUE;
		}
	}

	enumData->nonsorted.push_back(*deviceInstance);
	return DIENUM_CONTINUE;
}

BOOL CALLBACK enumCallbackA(LPCDIDEVICEINSTANCEA deviceInstance, LPVOID userData)
{
	return enumCallback<DIDEVICEINSTANCEA, string>(deviceInstance, userData);
}

BOOL CALLBACK enumCallbackW(LPCDIDEVICEINSTANCEW deviceInstance, LPVOID userData)
{
	return enumCallback<DIDEVICEINSTANCEW, wstring>(deviceInstance, userData);
}

HRESULT STDMETHODCALLTYPE HookEnumDevicesA(LPDIRECTINPUT8A This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	vector<string> &order = sortedControllersA();
	DeviceEnumData<DIDEVICEINSTANCEA> enumData;

	enumData.sorted.resize(order.size());

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
			PrintLog("DirectInput8Create - ANSI interface");
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
			PrintLog("DirectInput8Create - UNICODE interface");
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
	OutputDebugString(L"devreorder: Calling hooked DirectInput8Create");
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
	IClassFactory *cf;
	HRESULT hr = DirectInputModuleManager::Get().DllGetClassObject(rclsid, riid, (void**)&cf);

	if (hr != DI_OK) return hr;

	*ppv = cf;

	IDirectInput8 *realDI;
	hr = cf->lpVtbl->CreateInstance(cf, NULL, IID_IDirectInput8, (void**)&realDI);

	if (hr != DI_OK) return hr;

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
