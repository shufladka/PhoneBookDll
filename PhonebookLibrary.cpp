#include "pch.h"
#include "PhonebookLibrary.h"


// ������������� ����� ������
void InitializeSharedMemory(HWND hWnd) {
    bool isFirstInstance = false;

    // ������ ��� ��������� ������ ����������� ������
    HANDLE hMapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,               // ���������� ��������� ������
        NULL,                               // ��� ������
        PAGE_READWRITE,                     // ������ �������� ��� ������ � ������
        0,                                  // ������� 32 ���� ������� (0, �.�. ���������� MAX_MEMORY_SIZE)
        MAX_MEMORY_SIZE,                    // ������ 32 ���� �������
        sharedMemoryName                    // ��� �������
    );

    if (hMapping == NULL) {
        MessageBoxW(hWnd, L"�� ������� ������� ����� ������!", L"������", MB_OK | MB_ICONERROR);
        return;
    }

    // ���������, �������� �� ��� ������ �����������
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        isFirstInstance = false;  // ������ ��� �������
    }
    else {
        isFirstInstance = true;  // ������ ��������� ������ ������
    }

    // ����� ������ � �������� ������������ ��������
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(
        hMapping,
        FILE_MAP_ALL_ACCESS,  // ������ �� ������ � ������
        0,
        0,
        0
    );

    if (sharedMemory == NULL) {
        MessageBoxW(hWnd, L"�� ������� ���������� ������ � �������� ������������!", L"������", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }
}


// �������� ������ �� ���������� ����� � ����� ������
BOOL UploadToDatabase(HWND hwndListView, const char* filename) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = NULL;
    wchar_t* sharedMemory = NULL;

    // ����������� ������ ���� � �����
    WCHAR unicodeFilename[MAX_PATH] = { 0 };
    ConvertToUnicode(filename, unicodeFilename, sizeof(unicodeFilename) / sizeof(WCHAR));

    // ��������� ����
    hFile = CreateFileW(unicodeFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxW(hwndListView, L"�� ������� ������� ����!", L"������", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ��������� ������ �����
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0 || fileSize == INVALID_FILE_SIZE) {
        MessageBoxW(hwndListView, L"������ ����� ����������� ��� ����� ����!", L"������", MB_OK | MB_ICONERROR);
        CloseHandle(hFile);
        return FALSE;
    }

    // ������� ������ ������������� �����
    hMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, 0, sharedMemoryName);
    if (hMapping == NULL) {
        DWORD error = GetLastError();
        std::wstringstream ss;
        ss << L"������ �������� ������� ������������� �����. ���: " << error;
        MessageBoxW(hwndListView, ss.str().c_str(), L"������", MB_OK | MB_ICONERROR);
        CloseHandle(hFile);
        return FALSE;
    }

    // ���������� ���� � �������� ������������
    sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(hwndListView, L"�� ������� ���������� ���� � �������� ������������!", L"������", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return FALSE;
    }

    // ���������� ������ � ����� ������
    std::wifstream file(unicodeFilename);
    file.imbue(std::locale(".1251")); // ��� ANSI (Windows-1251)
    std::wstring fileContent;
    std::wstring line;
    while (std::getline(file, line)) {
        fileContent += line + L"\n";
    }
    memcpy(sharedMemory, fileContent.c_str(), fileContent.size() * sizeof(wchar_t));

    // ��������� �������
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    MessageBoxW(hwndListView, L"������ ������� ��������� � ����� ������!", L"����������", MB_OK);
    return TRUE;
}

// �������������� ������ �� ANSI � Unicode
void ConvertToUnicode(const char* ansiStr, WCHAR* unicodeStr, size_t unicodeStrSize) {
    MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, unicodeStr, unicodeStrSize);
}

// ������������ ��������
void UnloadFromDatabase() {
    if (fileData) UnmapViewOfFile(fileData);
    if (hMapping) CloseHandle(hMapping);
    if (hFile) CloseHandle(hFile);

    hFile = NULL;
    hMapping = NULL;
    fileData = nullptr;
    fileSize = 0;
    records.clear();
}

// ������������ ��������
void CleanupSharedMemory() {
    if (sharedMemory != NULL) {
        UnmapViewOfFile(sharedMemory);
    }

    if (hMapping != NULL) {
        CloseHandle(hMapping);
    }
}

// �������������� ������� ����������� � ������ ���������
std::vector<PhoneBookEntry> ParsePhoneBookData(const std::wstring& sharedMemoryContent) {
    std::vector<PhoneBookEntry> phonebookData;
    std::wistringstream stream(sharedMemoryContent);
    std::wstring line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue; // ���������� ������ ������

        std::wstringstream ss(line);
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

        // ������ ������ PhoneBookEntry
        PhoneBookEntry entry = {
            phone, lastName, firstName, patronymic,
            street, house, building, apartment
        };

        phonebookData.push_back(entry);
    }

    return phonebookData;
}

// ��������� ������ ������� ����������� �� ����� ������
std::vector<PhoneBookEntry> LoadPhoneBookDataFromSharedMemory(HWND hwndListView) {
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);

    if (!hMapping) {
        MessageBoxW(hwndListView, L"�� ������� ������������ � ����� ������!", L"������", MB_OK | MB_ICONERROR);
        return {};
    }

    wchar_t* sharedMemory = static_cast<wchar_t*>(MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0));
    if (!sharedMemory) {
        MessageBoxW(hwndListView, L"�� ������� ���������� ���� � �������� ������������!", L"������", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return {};
    }

    std::wstring sharedMemoryContent(sharedMemory);
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);

    return ParsePhoneBookData(sharedMemoryContent);
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
