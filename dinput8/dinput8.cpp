
#define CINTERFACE
#include "stdafx.h"
#include <dinput.h>
#include <list>

#include "dinput8.h"
#include "Common.h"
#include "Logger.h"
#include "Utils.h"
#include "DirectInputModuleManager.h"

using namespace std;

HRESULT(WINAPI *TrueDirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueCreateDeviceA) (LPDIRECTINPUT8A This, REFGUID rguid, LPDIRECTINPUTDEVICE8A *lplpDirectInputDeviceA, LPUNKNOWN pUnkOuter) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueCreateDeviceW) (LPDIRECTINPUT8W This, REFGUID rguid, LPDIRECTINPUTDEVICE8W *lplpDirectInputDeviceW, LPUNKNOWN pUnkOuter) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueGetPropertyA) (LPDIRECTINPUTDEVICE8A This, REFGUID rguidProp, LPDIPROPHEADER pdiph) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueGetPropertyW) (LPDIRECTINPUTDEVICE8W This, REFGUID rguidProp, LPDIPROPHEADER pdiph) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueGetDeviceInfoA) (LPDIRECTINPUTDEVICE8A This, LPDIDEVICEINSTANCEA pdidi) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueGetDeviceInfoW) (LPDIRECTINPUTDEVICE8W This, LPDIDEVICEINSTANCEW pdidi) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueEnumDevicesA) (LPDIRECTINPUT8A This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueEnumDevicesW) (LPDIRECTINPUT8W This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueSetCooperativeLevelA)(LPDIRECTINPUTDEVICE8A This, HWND hWnd, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *TrueSetCooperativeLevelW)(LPDIRECTINPUTDEVICE8W This, HWND hWnd, DWORD dwFlags) = nullptr;

LPDIENUMDEVICESCALLBACKA TrueCallbackA = nullptr;
LPDIENUMDEVICESCALLBACKW TrueCallbackW = nullptr;

HRESULT(STDMETHODCALLTYPE *CreateDeviceA) (LPDIRECTINPUT8A This, REFGUID rguid, LPDIRECTINPUTDEVICE8A *lplpDirectInputDeviceA, LPUNKNOWN pUnkOuter) = nullptr;
HRESULT(STDMETHODCALLTYPE *CreateDeviceW) (LPDIRECTINPUT8W This, REFGUID rguid, LPDIRECTINPUTDEVICE8W *lplpDirectInputDeviceW, LPUNKNOWN pUnkOuter) = nullptr;
HRESULT(STDMETHODCALLTYPE *GetPropertyA) (LPDIRECTINPUTDEVICE8A This, REFGUID rguidProp, LPDIPROPHEADER pdiph) = nullptr;
HRESULT(STDMETHODCALLTYPE *GetPropertyW) (LPDIRECTINPUTDEVICE8W This, REFGUID rguidProp, LPDIPROPHEADER pdiph) = nullptr;
HRESULT(STDMETHODCALLTYPE *GetDeviceInfoA) (LPDIRECTINPUTDEVICE8A This, LPDIDEVICEINSTANCEA pdidi) = nullptr;
HRESULT(STDMETHODCALLTYPE *GetDeviceInfoW) (LPDIRECTINPUTDEVICE8W This, LPDIDEVICEINSTANCEW pdidi) = nullptr;
HRESULT(STDMETHODCALLTYPE *EnumDevicesA) (LPDIRECTINPUT8A This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *EnumDevicesW) (LPDIRECTINPUT8W This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *SetCooperativeLevelA)(LPDIRECTINPUTDEVICE8A This, HWND hWnd, DWORD dwFlags) = nullptr;
HRESULT(STDMETHODCALLTYPE *SetCooperativeLevelW)(LPDIRECTINPUTDEVICE8W This, HWND hWnd, DWORD dwFlags) = nullptr;
/*
BOOL FAR PASCAL HookEnumCallbackA(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
	if (!InputHookManager::Get().GetInputHook().GetState(InputHook::HOOK_DI)) return TrueCallbackA(lpddi, pvRef);
	PrintLog("EnumCallbackA");

	if (!DeviceCheck(lpddi)) return TrueCallbackA(lpddi, pvRef);

	if (InputHookManager::Get().GetInputHook().GetState(InputHook::HOOK_STOP)) return DIENUM_STOP;

	for (auto padcfg = InputHookManager::Get().GetInputHook().begin(); padcfg != InputHookManager::Get().GetInputHook().end(); ++padcfg)
	{
		if (IsEqualGUID(padcfg->GetProductGUID(), lpddi->guidProduct))
		{
			DIDEVICEINSTANCEA fakedev = *lpddi;
			if (InputHookManager::Get().GetInputHook().GetState(InputHook::HOOK_PIDVID))
			{
				std::string strTrueguidProduct;
				std::string strHookguidProduct;

				GUIDtoString(&strTrueguidProduct, fakedev.guidProduct);
				fakedev.guidProduct.Data1 = InputHookManager::Get().GetInputHook().GetFakePIDVID();
				GUIDtoString(&strHookguidProduct, fakedev.guidProduct);

				PrintLog("GUID change:");
				PrintLog("%s", strTrueguidProduct.c_str());
				PrintLog("%s", strHookguidProduct.c_str());
			}

			// This should not be required
			// fakedev.dwDevType = (MAKEWORD(DI8DEVTYPE_GAMEPAD, DI8DEVTYPEGAMEPAD_STANDARD) | DIDEVTYPE_HID); //66069 == 0x00010215
			// fakedev.wUsage = 0x05;
			// fakedev.wUsagePage = 0x01;

			if (InputHookManager::Get().GetInputHook().GetState(InputHook::HOOK_NAME))
			{
				std::string OldProductName = fakedev.tszProductName;
				std::string OldInstanceName = fakedev.tszInstanceName;

				strcpy_s(fakedev.tszProductName, "XBOX 360 For Windows (Controller)");
				strcpy_s(fakedev.tszInstanceName, "XBOX 360 For Windows (Controller)");

				PrintLog("Product Name change:");
				PrintLog("\"%s\"", OldProductName.c_str());
				PrintLog("\"%s\"", fakedev.tszProductName);

				PrintLog("Instance Name change:");
				PrintLog("\"%s\"", OldInstanceName.c_str());
				PrintLog("\"%s\"", fakedev.tszInstanceName);
			}

			return TrueCallbackA(&fakedev, pvRef);
		}
	}
	return TrueCallbackA(lpddi, pvRef);
}

BOOL FAR PASCAL HookEnumCallbackW(LPCDIDEVICEINSTANCEW lpddi, LPVOID pvRef)
{
	if (!InputHookManager::Get().GetInputHook().GetState(InputHook::HOOK_DI)) return TrueCallbackW(lpddi, pvRef);
	PrintLog("EnumCallbackW");

	if (!DeviceCheck(lpddi)) return TrueCallbackW(lpddi, pvRef);

	if (InputHookManager::Get().GetInputHook().GetState(InputHook::HOOK_STOP)) return DIENUM_STOP;

	for (auto padcfg = InputHookManager::Get().GetInputHook().begin(); padcfg != InputHookManager::Get().GetInputHook().end(); ++padcfg)
	{
		if (IsEqualGUID(padcfg->GetProductGUID(), lpddi->guidProduct))
		{
			DIDEVICEINSTANCEW fakedev = *lpddi;
			if (InputHookManager::Get().GetInputHook().GetState(InputHook::HOOK_PIDVID))
			{
				std::wstring strTrueguidProduct;
				std::wstring strHookguidProduct;

				GUIDtoString(&strTrueguidProduct, fakedev.guidProduct);
				fakedev.guidProduct.Data1 = InputHookManager::Get().GetInputHook().GetFakePIDVID();
				GUIDtoString(&strHookguidProduct, fakedev.guidProduct);

				PrintLog("GUID change:");
				PrintLog("%ls", strTrueguidProduct.c_str());
				PrintLog("%ls", strHookguidProduct.c_str());
			}

			// This should not be required
			// fakedev.dwDevType = (MAKEWORD(DI8DEVTYPE_GAMEPAD, DI8DEVTYPEGAMEPAD_STANDARD) | DIDEVTYPE_HID); //66069 == 0x00010215
			// fakedev.wUsage = 0x05;
			// fakedev.wUsagePage = 0x01;

			if (InputHookManager::Get().GetInputHook().GetState(InputHook::HOOK_NAME))
			{
				std::wstring OldProductName(fakedev.tszProductName);
				std::wstring OldInstanceName(fakedev.tszInstanceName);

				wcscpy_s(fakedev.tszProductName, L"XBOX 360 For Windows (Controller)");
				wcscpy_s(fakedev.tszInstanceName, L"XBOX 360 For Windows (Controller)");

				PrintLog("Product Name change:");
				PrintLog("\"%ls\"", OldProductName.c_str());
				PrintLog("\"%ls\"", fakedev.tszProductName);

				PrintLog("Instance Name change:");
				PrintLog("\"%ls\"", OldInstanceName.c_str());
				PrintLog("\"%ls\"", fakedev.tszInstanceName);
			}

			return TrueCallbackW(&fakedev, pvRef);
		}
	}

	return TrueCallbackW(lpddi, pvRef);
}
*/


vector<wstring> & sortedControllersW()
{
	static vector<wstring> result;
	static bool needToInitialize = true;

	if (needToInitialize) {
		result.push_back(L"vJoy Device");

		needToInitialize = false;
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
		result.push_back(L"Wireless Controller");

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

bool stringsAreEqual(const string &a, const char *b)
{
	return strcmp(a.c_str(), b) == 0;
}

bool stringsAreEqual(const wstring &a, const WCHAR *b)
{
	return lstrcmpW(a.c_str(), b) == 0;
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
	PrintLog("calling enumCallbackA");
	return enumCallback<DIDEVICEINSTANCEA, string>(deviceInstance, userData);
}

BOOL CALLBACK enumCallbackW(LPCDIDEVICEINSTANCEW deviceInstance, LPVOID userData)
{
	PrintLog("calling enumCallbackW");
	return enumCallback<DIDEVICEINSTANCEW, wstring>(deviceInstance, userData);
}

HRESULT STDMETHODCALLTYPE HookEnumDevicesA(LPDIRECTINPUT8A This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	PrintLog("IDirectInput8A::EnumDevicesA");
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
	PrintLog("IDirectInput8W::EnumDevicesW");
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

extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
	OutputDebugString(L"Testing DirectInput8Create!");

	HRESULT hr = DirectInputModuleManager::Get().DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	PrintLog("DirectInput8Create");

	if (hr != DI_OK) return hr;

	if (IsEqualIID(riidltf, IID_IDirectInput8A))
	{
		LPDIRECTINPUT8A pDIA = static_cast<LPDIRECTINPUT8A>(*ppvOut);

		if (pDIA)
		{
			PrintLog("DirectInput8Create - ANSI interface");
			/*if (pDIA->lpVtbl->CreateDevice)
			{
				CreateDeviceA = pDIA->lpVtbl->CreateDevice;
				IH_CreateHook(CreateDeviceA, HookCreateDeviceA, &TrueCreateDeviceA);
				IH_EnableHook(CreateDeviceA);
			}*/
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
		LPDIRECTINPUT8W pDIW = static_cast<LPDIRECTINPUT8W>(*ppvOut);

		if (pDIW)
		{
			PrintLog("DirectInput8Create - UNICODE interface");
			/*if (pDIW->lpVtbl->CreateDevice)
			{
				CreateDeviceW = pDIW->lpVtbl->CreateDevice;
				IH_CreateHook(CreateDeviceW, HookCreateDeviceW, &TrueCreateDeviceW);
				IH_EnableHook(CreateDeviceW);
			}*/
			if (pDIW->lpVtbl->EnumDevices)
			{
				EnumDevicesW = pDIW->lpVtbl->EnumDevices;
				IH_CreateHook(EnumDevicesW, HookEnumDevicesW, &TrueEnumDevicesW);
				IH_EnableHook(EnumDevicesW);
			}
		}
	}

	return hr;

	//return DirectInputModuleManager::Get().DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

extern "C" HRESULT WINAPI DllCanUnloadNow(void)
{
	return DirectInputModuleManager::Get().DllCanUnloadNow();
}

extern "C" HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv)
{
	return DirectInputModuleManager::Get().DllGetClassObject(rclsid, riid, ppv);
}

extern "C" HRESULT WINAPI DllRegisterServer(void)
{
	return DirectInputModuleManager::Get().DllRegisterServer();
}

extern "C" HRESULT WINAPI DllUnregisterServer(void)
{
	return DirectInputModuleManager::Get().DllUnregisterServer();
}
