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
        wstringstream ss;
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

    // �������� ����� ������ ����� ������� ����� ������
    memset(sharedMemory, 0, fileSize);  // ������� ������ ����� ������� ����� ������

    // ���������� ������ � ����� ������
    wifstream file(unicodeFilename);
    file.imbue(locale(".1251")); // ��� ANSI (Windows-1251)
    wstring fileContent;
    wstring line;
    while (getline(file, line)) {
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
void CleanupResources() {
    if (fileData) {
        UnmapViewOfFile(fileData);
        fileData = nullptr;
    }

    if (hMapping) {
        CloseHandle(hMapping);
        hMapping = NULL;
    }

    if (hFile) {
        CloseHandle(hFile);
        hFile = NULL;
    }

    sharedMemory = NULL;
    fileSize = 0;
    phonebookData.clear();
}

void ClearSharedMemory() {
    if (sharedMemory != NULL) {
        // �������� ���������� ������
        memset(sharedMemory, 0, fileSize);

        // ������� ����������� ������
        UnmapViewOfFile(sharedMemory);
        sharedMemory = NULL;
    }

    if (hMapping != NULL) {
        // ��������� ������ �������������
        CloseHandle(hMapping);
        hMapping = NULL;
    }

    // ��������� ������� ������ �������������
    HANDLE hTempMapping = OpenFileMappingW(FILE_MAP_WRITE, FALSE, sharedMemoryName);
    if (hTempMapping != NULL) {
        CloseHandle(hTempMapping);
        DeleteFileW(sharedMemoryName); // �������� �����, ���� �� ������ � ��������
    }

    // ���������� ����������
    fileSize = 0;
}

// �������� �� ������� ������ � ����� ������
bool IsSharedMemoryEmpty(HWND hwnd) {

    // ������������ � ������������� ������� ����� ������
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(hwnd, L"�� ������� ������������ � ����� ������!", L"������", MB_OK | MB_ICONERROR);
        return true; // ���� ������ ���, ������� � "������"
    }

    // ����� ������ � �������� ������������ ��������
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(hwnd, L"�� ������� ���������� ������ � �������� ������������!", L"������", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return true; // ���� �� ������� ����������, ������� � ������
    }

    // ���������, ���� �� ������ (�������� �� ������ ������)
    bool isEmpty = (sharedMemory[0] == L'\0');

    // ��������� �������
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);

    return isEmpty;
}

// �������������� ������� ����������� � ������ ���������
vector<PhoneBookEntry> ParsePhoneBookData(const wstring& sharedMemoryContent) {
    vector<PhoneBookEntry> phonebookData;
    wistringstream stream(sharedMemoryContent);
    wstring line;

    while (getline(stream, line)) {
        if (line.empty()) continue; // ���������� ������ ������

        wstringstream ss(line);
        wstring phone, lastName, firstName, patronymic, street, house, building, apartment;

        // ��������� ������ �� ������� 'delimeter'
        getline(ss, phone, delimeter);
        getline(ss, lastName, delimeter);
        getline(ss, firstName, delimeter);
        getline(ss, patronymic, delimeter);
        getline(ss, street, delimeter);
        getline(ss, house, delimeter);
        getline(ss, building, delimeter);
        getline(ss, apartment, delimeter);

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
vector<PhoneBookEntry> LoadDatabaseFromMemory(HWND hwndListView) {
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

    wstring sharedMemoryContent(sharedMemory);
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);

    return ParsePhoneBookData(sharedMemoryContent);
}

// ������� ������ ������� �� ���������� ����
vector<PhoneBookEntry> SearchByField(const wstring& value, const vector<PhoneBookEntry>& phonebookData, int fieldIndex) {
    vector<PhoneBookEntry> results;

    for (const auto& record : phonebookData) {

        // ��������� ���� �� ������ �������
        bool match = false;

        switch (fieldIndex) {
        case 0: // ����� �� ���� �����
            match = record.phone.find(value) != wstring::npos ||
                record.lastName.find(value) != wstring::npos ||
                record.firstName.find(value) != wstring::npos ||
                record.patronymic.find(value) != wstring::npos ||
                record.street.find(value) != wstring::npos ||
                record.house.find(value) != wstring::npos ||
                record.building.find(value) != wstring::npos ||
                record.apartment.find(value) != wstring::npos;
            break;
        case 1: // �������
            match = record.phone.find(value) != wstring::npos;
            break;
        case 2: // �������
            match = record.lastName.find(value) != wstring::npos;
            break;
        case 3: // ���
            match = record.firstName.find(value) != wstring::npos;
            break;
        case 4: // ��������
            match = record.patronymic.find(value) != wstring::npos;
            break;
        case 5: // �����
            match = record.street.find(value) != wstring::npos;
            break;
        case 6: // ���
            match = record.house.find(value) != wstring::npos;
            break;
        case 7: // ������
            match = record.building.find(value) != wstring::npos;
            break;
        case 8: // ��������
            match = record.apartment.find(value) != wstring::npos;
            break;
        }

        // ���� ������� ����������, ��������� ������ � ����������
        if (match) {
            results.push_back(record);
        }
    }

    return results; // ���������� ��������� ������
}
