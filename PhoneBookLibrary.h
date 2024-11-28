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

// Глобальные переменные для работы с проецированием файла
static HANDLE hFile = NULL;
static HANDLE hMapping = NULL;
static wchar_t* fileData = nullptr;
static size_t fileSize = 0;

bool fileLoaded = false; // Глобальный флаг, который указывает, загружен ли файл


const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";
wchar_t* sharedMemory = NULL;
constexpr size_t MAX_MEMORY_SIZE = 65536; // Максимальный размер памяти (в байтах)

#define MAX_ENTRY_SIZE 512  // Максимальный размер одной записи
#define MAX_RECORDS 100     // Максимальное количество записей

// Общий размер для одной записи:
#define MAX_RECORD_SIZE (MAX_ENTRY_SIZE * NUM_FIELDS)

// Структура для хранения данных из справочника
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

//static std::vector<PhoneBookEntry> records;
std::vector<PhoneBookEntry> phonebookData;

extern "C" DLL_LIB_API void InitializeSharedMemory(HWND hWnd);
extern "C" DLL_LIB_API BOOL UploadToDatabase(HWND hwndListView, const char* filename);
extern "C" DLL_LIB_API void CleanupResources();

extern "C" DLL_LIB_API void ClearSharedMemory();

extern "C" DLL_LIB_API std::vector<PhoneBookEntry> ParsePhoneBookData(const std::wstring& sharedMemoryContent);
extern "C" DLL_LIB_API std::vector<PhoneBookEntry> LoadDatabaseFromMemory(HWND hwndListView);
extern "C" DLL_LIB_API std::vector<PhoneBookEntry> SearchByPhone(const std::wstring& phone, const std::vector<PhoneBookEntry>& phonebookData);
extern "C" DLL_LIB_API std::vector<PhoneBookEntry> GetPhoneList(const std::vector<PhoneBookEntry>& phonebookData);

void ConvertToUnicode(const char* ansiStr, WCHAR* unicodeStr, size_t unicodeStrSize);
