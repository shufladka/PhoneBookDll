#include "pch.h"
#include "PhonebookLibrary.h"

// ��������� ��� �������� ������ ������ �� �����������
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

// ���������� ���������� ��� ������ � �������������� �����
static HANDLE hFile = NULL;
static HANDLE hMapping = NULL;
static wchar_t* fileData = nullptr;
static size_t fileSize = 0;

static std::vector<Record> records;

// �������� ���� ������ �� �����
bool LoadDatabase(const WCHAR* filename) {
    // ��������� ���� ��� ������
    hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    // �������� ������ �����
    fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return false;
    }

    // ������ ����������� �����
    hMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapping) {
        CloseHandle(hFile);
        return false;
    }

    // ����� ���� � ������
    fileData = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!fileData) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }

    // ������ ������ �� �����
    records.clear(); // ������� ������ �� ������ ���������� ������
    std::wistringstream fileStream(std::wstring(fileData, fileSize / sizeof(wchar_t)));
    std::wstring line;
    while (std::getline(fileStream, line)) {
        std::wistringstream lineStream(line);
        Record record;

        std::getline(lineStream, record.phone, L',');
        std::getline(lineStream, record.lastName, L',');
        std::getline(lineStream, record.firstName, L',');
        std::getline(lineStream, record.patronymic, L',');
        std::getline(lineStream, record.street, L',');
        std::getline(lineStream, record.house, L',');
        std::getline(lineStream, record.building, L',');
        std::getline(lineStream, record.apartment, L',');

        records.push_back(record);
    }

    return true;
}

// ������������ ��������
void UnloadDatabase() {
    if (fileData) UnmapViewOfFile(fileData);
    if (hMapping) CloseHandle(hMapping);
    if (hFile) CloseHandle(hFile);

    hFile = NULL;
    hMapping = NULL;
    fileData = nullptr;
    fileSize = 0;
    records.clear();
}

// ����� ������ �� ������ ��������
const wchar_t* SearchByPhone(const wchar_t* phone) {
    static std::wstring result;

    for (const auto& record : records) {
        if (record.phone == phone) {
            result = record.lastName + L" " + record.firstName + L" " + record.patronymic + L", " +
                record.street + L" " + record.house + L", " + record.building + L", " + record.apartment;
            return result.c_str();
        }
    }

    return L"Not found";
}

// ������� ��� ��������� ������ ���� �������
void GetPhoneList(std::vector<std::wstring>& phoneList) {
    phoneList.clear();
    for (const auto& record : records) {
        phoneList.push_back(std::wstring(record.phone.begin(), record.phone.end()));
    }
}