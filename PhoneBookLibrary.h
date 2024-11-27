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

// ���������� ���������� ��� ������ � �������������� �����
static HANDLE hFile = NULL;
static HANDLE hMapping = NULL;
static wchar_t* fileData = nullptr;
static size_t fileSize = 0;

wchar_t* sharedMemory = NULL;
constexpr size_t MAX_MEMORY_SIZE = 65536; // ������������ ������ ������ (� ������)

#define MAX_ENTRY_SIZE 512  // ������������ ������ ����� ������
#define MAX_RECORDS 100     // ������������ ���������� �������

// ����� ������ ��� ����� ������:
#define MAX_RECORD_SIZE (MAX_ENTRY_SIZE * NUM_FIELDS)

//static std::vector<Record> records;

// ��������� ��� �������� ������ �� �����������
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

static std::vector<PhoneBookEntry> records;


extern "C" DLL_LIB_API void UnloadDatabase();
extern "C" DLL_LIB_API std::vector<PhoneBookEntry> SearchByPhone(const std::wstring& phone, const std::vector<PhoneBookEntry>& phonebookData);
extern "C" DLL_LIB_API std::vector<PhoneBookEntry> GetPhoneList(const std::vector<PhoneBookEntry>& phonebookData);
