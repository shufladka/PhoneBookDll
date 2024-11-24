#include "pch.h"
#include "PhonebookLibrary.h"

// ������� ��� ������ ������ �� �����
bool ReadPhoneBookData(const WCHAR* filename, std::vector<PhoneBookEntry>& entries) {

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

    std::wifstream file(filename);  // ��������� ���� ��� ������
    if (!file.is_open()) {
        return false;  // ���� �� ������� ������� ����
    }

    std::wstring line;
    while (std::getline(file, line)) {
        std::wstringstream ss(line);  // ������ ��������� ����� ��� ������� ������
        std::wstring phone, lastName, firstName, patronymic, street, house, building, apartment;

        // ��������� ������ �� ������� ';'
        std::getline(ss, phone, L';');
        std::getline(ss, lastName, L';');
        std::getline(ss, firstName, L';');
        std::getline(ss, patronymic, L';');
        std::getline(ss, street, L';');
        std::getline(ss, house, L';');
        std::getline(ss, building, L';');
        std::getline(ss, apartment, L';');

        // ��������� ������ � ������
        PhoneBookEntry entry = { phone, lastName, firstName, patronymic, street, house, building, apartment };
        entries.push_back(entry);
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
