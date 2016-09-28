#pragma once

#include <string>
#include <Windows.h>
#include "Types.h"

HMODULE LoadLibrarySystem(const std::string& library_name, std::string* out_path);
HMODULE LoadLibrarySystem(const std::wstring& library_name, std::wstring* out_path);
HMODULE LoadLibraryCurrent(const std::string& library_name, std::string* out_path);
HMODULE LoadLibraryCurrent(const std::wstring& library_name, std::wstring* out_path);

inline HMODULE& CurrentModule()
{
	static HMODULE hModule = 0;
	if (!hModule)
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&hModule, &hModule);
	return hModule;
}

bool FileExist(const std::string& path);
bool FileExist(const std::wstring& path);
bool CheckCommonDirectory(std::string* outpath, const std::string& dirname);
bool CheckCommonDirectory(std::wstring* outpath, const std::wstring& dirname);
bool FullPathFromPath(std::string* fullpath, const std::string& name);
bool FullPathFromPath(std::wstring* fullpath, const std::wstring& name);
bool StringPathAppend(std::string* path, const std::string& more);
bool StringPathAppend(std::wstring* path, const std::wstring& more);
bool ModulePath(std::string* out, HMODULE hModule = NULL);
bool ModulePath(std::wstring* out, HMODULE hModule = NULL);
bool ModuleDirectory(std::string* out, HMODULE hModule = NULL);
bool ModuleDirectory(std::wstring* out, HMODULE hModule = NULL);
bool ModuleFileName(std::string* out, HMODULE hModule = NULL);
bool ModuleFileName(std::wstring* out, HMODULE hModule = NULL);
std::wstring thisModuleDirectory();
std::wstring getSystemDirectoryString();
