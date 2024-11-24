#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include <fstream>

#ifdef DLL_LIB_EXPORTS
#define DLL_LIB_API __declspec(dllexport)
#else
#define DLL_LIB_API __declspec(dllimport)
#endif

// Структура для хранения строки данных из справочника
struct Record {
    std::wstring phone;
    std::wstring lastName;
    std::wstring firstName;
    std::wstring patronymic;
    std::wstring street;
    std::wstring house;
    std::wstring building;
    std::wstring apartment;
};

// Глобальные переменные для работы с проецированием файла
static HANDLE hFile = NULL;
static HANDLE hMapping = NULL;
static wchar_t* fileData = nullptr;
static size_t fileSize = 0;

static std::vector<Record> records;

// Структура для хранения информации о пользователе
struct PhoneBookEntry {
    std::wstring phone;
    std::wstring lastName;
    std::wstring firstName;
    std::wstring patronymic;
    std::wstring street;
    std::wstring house;
    std::wstring building;
    std::wstring apartment;
};

extern "C" DLL_LIB_API bool LoadDatabase(const WCHAR* filename);
extern "C" DLL_LIB_API void UnloadDatabase();
extern "C" DLL_LIB_API const wchar_t* SearchByPhone(const wchar_t* phone);
extern "C" DLL_LIB_API void GetPhoneList(std::vector<std::wstring>& phoneList);
extern "C" DLL_LIB_API bool ReadPhoneBookData(const WCHAR* filename, std::vector<PhoneBookEntry>& entries);