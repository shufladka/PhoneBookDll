#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <windows.h>

#ifdef DLL_LIB_EXPORTS
#define DLL_LIB_API __declspec(dllexport)
#else
#define DLL_LIB_API __declspec(dllimport)
#endif


extern "C" DLL_LIB_API bool LoadDatabase(const WCHAR* filename);
extern "C" DLL_LIB_API void UnloadDatabase();
extern "C" DLL_LIB_API const wchar_t* SearchByPhone(const wchar_t* phone);
extern "C" DLL_LIB_API void GetPhoneList(std::vector<std::wstring>& phoneList);
