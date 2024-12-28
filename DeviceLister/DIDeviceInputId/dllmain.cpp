
#include "pch.h"
#include <initguid.h>
#include <string>
#include <dinput.h>
#include <Devpkey.h>
#include <Cfgmgr32.h>

std::wstring getDeviceInstanceIdProperty(const DIPROPGUIDANDPATH &iap)
{
	DEVPROPTYPE propertyType;
	ULONG propertySize = 0;
	CONFIGRET cr = CM_Get_Device_Interface_PropertyW(iap.wszPath, &DEVPKEY_Device_InstanceId, &propertyType, nullptr, &propertySize, 0);

	if (cr != CR_BUFFER_SMALL) {
		OutputDebugStringA("CM_Get_Device_Interface_PropertyW 1 failed");
		return std::wstring();
	}

	std::wstring deviceId;
	deviceId.resize(propertySize);
	cr = ::CM_Get_Device_Interface_PropertyW(iap.wszPath, &DEVPKEY_Device_InstanceId, &propertyType, (PBYTE)deviceId.data(), &propertySize, 0);

	if (cr != CR_SUCCESS) {
		OutputDebugStringA("CM_Get_Device_Interface_PropertyW 2 failed");
		return std::wstring();
	}

	return deviceId;
}

extern "C" {

// Function that takes a string and returns a string
__declspec(dllexport) const wchar_t * GetDeviceInstanceID(const wchar_t *instanceGUIDStr)
{
	wchar_t *output = nullptr;
	HRESULT hr;
	LPDIRECTINPUT8 di;

	hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&di, nullptr);

	if (FAILED(hr)) {
		OutputDebugStringA("DirectInput8Create failed");
		return nullptr;
	}

	GUID instanceGUID;

	if (UuidFromStringW((RPC_WSTR)instanceGUIDStr, &instanceGUID) != RPC_S_OK) {
		OutputDebugStringA("UuidFromStringA failed");
		OutputDebugStringW(instanceGUIDStr);
		return nullptr;
	}

	IDirectInputDevice8W* device;
	IDirectInput_CreateDevice(di, instanceGUID, &device, NULL);

	DIPROPGUIDANDPATH iap = {};
	iap.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
	iap.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	iap.diph.dwHow = DIPH_DEVICE;

	std::wstring deviceId;

	if (SUCCEEDED(IDirectInputDevice_GetProperty(device, DIPROP_GUIDANDPATH, &iap.diph))) {
		deviceId = getDeviceInstanceIdProperty(iap);
	} else {
		OutputDebugStringA("IDirectInputDevice_GetProperty failed");
	}

	IDirectInputDevice_Release(device);

	if (deviceId.size() == 0) {
		return nullptr;
	}

	size_t outputSize = deviceId.size() + 1;
	output = new wchar_t[outputSize];
	wcscpy_s(output, outputSize, deviceId.c_str());

    return output;
}

}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

