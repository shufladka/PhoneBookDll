#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include <fstream>

using namespace std;

#ifdef DLL_LIB_EXPORTS
#define DLL_LIB_API __declspec(dllexport)
#else
#define DLL_LIB_API __declspec(dllimport)
#endif

static HANDLE hFile = NULL;
static HANDLE hMapping = NULL;
static wchar_t* fileData = nullptr;
static size_t fileSize = 0;

const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";
wchar_t* sharedMemory = NULL;
constexpr size_t MAX_MEMORY_SIZE = 65536;                   // ћаксимальный размер пам€ти (в байтах)
#define MAX_ENTRY_SIZE 512                                  // ћаксимальный размер одной записи
#define MAX_RECORDS 100                                     // ћаксимальное количество записей
#define MAX_RECORD_SIZE (MAX_ENTRY_SIZE * NUM_FIELDS)       // ќбщий размер дл€ одной записи

// —труктура дл€ хранени€ данных из справочника
struct PhoneBookEntry {
    wstring phone;
    wstring lastName;
    wstring firstName;
    wstring patronymic;
    wstring street;
    wstring house;
    wstring building;
    wstring apartment;
};

vector<PhoneBookEntry> phonebookData;      // √лобальна€ переменна€ дл€ хранени€ десериализованного массива данных из справочника

extern "C" DLL_LIB_API  void    InitializeSharedMemory(HWND hWnd);
extern "C" DLL_LIB_API  BOOL    UploadToDatabase(HWND hwndListView, const char* filename);
extern "C" DLL_LIB_API  void    CleanupResources();

extern "C" DLL_LIB_API  bool    IsSharedMemoryEmpty(HWND hwnd);
extern "C" DLL_LIB_API  void    ClearSharedMemory();

extern "C" DLL_LIB_API  vector<PhoneBookEntry>  ParsePhoneBookData(const wstring& sharedMemoryContent);
extern "C" DLL_LIB_API  vector<PhoneBookEntry>  LoadDatabaseFromMemory(HWND hwndListView);
extern "C" DLL_LIB_API  vector<PhoneBookEntry>  SearchByPhone(const wstring& phone, const vector<PhoneBookEntry>& phonebookData);

void ConvertToUnicode(const char* ansiStr, WCHAR* unicodeStr, size_t unicodeStrSize);
