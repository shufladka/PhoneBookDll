#include "pch.h"
#include "PhonebookLibrary.h"

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

// ������� ������ ������� �� ������ ��������
std::vector<PhoneBookEntry> SearchByPhone(const std::wstring& phone, const std::vector<PhoneBookEntry>& phonebookData) {
    std::vector<PhoneBookEntry> results;

    // �������� �� ���� ������� � ���� ���������� �� ������ (���������)
    for (const auto& record : phonebookData) {
        if (record.phone.find(phone) != std::wstring::npos) { // ���������, ���������� �� phone � record.phone
            results.push_back(record); // ��������� � ����������, ���� ������� ����������
        }
    }

    // ���������� ��������� ������ (������ ����� ���� ������, ���� ������ �� �������)
    return results;
}

// ������� ��� ��������� ������ ���� �������
std::vector<PhoneBookEntry> GetPhoneList(const std::vector<PhoneBookEntry>& phonebookData) {
    std::vector<PhoneBookEntry> results;

    // �������� �� ���� �������
    for (const auto& record : phonebookData) {
        results.push_back(record); // ��������� � ����������
    }

    // ���������� ��������� ������ (������ ����� ���� ������, ���� ������ �� �������)
    return results;
}
