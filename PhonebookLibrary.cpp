#include "pch.h"
#include "PhonebookLibrary.h"


// Инициализация общей памяти
void InitializeSharedMemory(HWND hWnd) {
    bool isFirstInstance = false;

    // Создаём или открываем объект отображения памяти
    HANDLE hMapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,               // Используем системную память
        NULL,                               // Без защиты
        PAGE_READWRITE,                     // Память доступна для чтения и записи
        0,                                  // Высокие 32 бита размера (0, т.к. используем MAX_MEMORY_SIZE)
        MAX_MEMORY_SIZE,                    // Низкие 32 бита размера
        sharedMemoryName                    // Имя объекта
    );

    if (hMapping == NULL) {
        MessageBoxW(hWnd, L"Не удалось создать общую память!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Проверяем, является ли это первым экземпляром
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        isFirstInstance = false;  // Память уже создана
    }
    else {
        isFirstInstance = true;  // Первый экземпляр создал память
    }

    // Мапим память в адресное пространство процесса
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(
        hMapping,
        FILE_MAP_ALL_ACCESS,  // Доступ на чтение и запись
        0,
        0,
        0
    );

    if (sharedMemory == NULL) {
        MessageBoxW(hWnd, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }
}


// Выгрузка данных из текстового файла в общую память
BOOL UploadToDatabase(HWND hwndListView, const char* filename) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = NULL;
    wchar_t* sharedMemory = NULL;

    // Преобразуем строку пути к файлу
    WCHAR unicodeFilename[MAX_PATH] = { 0 };
    ConvertToUnicode(filename, unicodeFilename, sizeof(unicodeFilename) / sizeof(WCHAR));

    // Открываем файл
    hFile = CreateFileW(unicodeFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxW(hwndListView, L"Не удалось открыть файл!", L"Ошибка", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // Проверяем размер файла
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0 || fileSize == INVALID_FILE_SIZE) {
        MessageBoxW(hwndListView, L"Размер файла некорректен или равен нулю!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hFile);
        return FALSE;
    }

    // Создаем объект проецирования файла
    hMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, 0, sharedMemoryName);
    if (hMapping == NULL) {
        DWORD error = GetLastError();
        std::wstringstream ss;
        ss << L"Ошибка создания объекта проецирования файла. Код: " << error;
        MessageBoxW(hwndListView, ss.str().c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hFile);
        return FALSE;
    }

    // Отображаем файл в адресное пространство
    sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(hwndListView, L"Не удалось отобразить файл в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return FALSE;
    }

    // Записываем данные в общую память
    std::wifstream file(unicodeFilename);
    file.imbue(std::locale(".1251")); // Для ANSI (Windows-1251)
    std::wstring fileContent;
    std::wstring line;
    while (std::getline(file, line)) {
        fileContent += line + L"\n";
    }
    memcpy(sharedMemory, fileContent.c_str(), fileContent.size() * sizeof(wchar_t));

    // Закрываем ресурсы
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    MessageBoxW(hwndListView, L"Данные успешно загружены в общую память!", L"Информация", MB_OK);
    return TRUE;
}

// Преобразование строки из ANSI в Unicode
void ConvertToUnicode(const char* ansiStr, WCHAR* unicodeStr, size_t unicodeStrSize) {
    MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, unicodeStr, unicodeStrSize);
}

// Освобождение ресурсов
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

// Освобождение ресурсов
void CleanupSharedMemory() {
    if (sharedMemory != NULL) {
        UnmapViewOfFile(sharedMemory);
    }

    if (hMapping != NULL) {
        CloseHandle(hMapping);
    }
}

// Десериализация записей справочника в массив сущностей
std::vector<PhoneBookEntry> ParsePhoneBookData(const std::wstring& sharedMemoryContent) {
    std::vector<PhoneBookEntry> phonebookData;
    std::wistringstream stream(sharedMemoryContent);
    std::wstring line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue; // Пропускаем пустые строки

        std::wstringstream ss(line);
        std::wstring phone, lastName, firstName, patronymic, street, house, building, apartment;

        // Разделяем строку по символу ';'
        std::getline(ss, phone, L';');
        std::getline(ss, lastName, L';');
        std::getline(ss, firstName, L';');
        std::getline(ss, patronymic, L';');
        std::getline(ss, street, L';');
        std::getline(ss, house, L';');
        std::getline(ss, building, L';');
        std::getline(ss, apartment, L';');

        // Создаём объект PhoneBookEntry
        PhoneBookEntry entry = {
            phone, lastName, firstName, patronymic,
            street, house, building, apartment
        };

        phonebookData.push_back(entry);
    }

    return phonebookData;
}

// Получение списка записей справочника из общей памяти
std::vector<PhoneBookEntry> LoadPhoneBookDataFromSharedMemory(HWND hwndListView) {
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);

    if (!hMapping) {
        MessageBoxW(hwndListView, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
        return {};
    }

    wchar_t* sharedMemory = static_cast<wchar_t*>(MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0));
    if (!sharedMemory) {
        MessageBoxW(hwndListView, L"Не удалось отобразить файл в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return {};
    }

    std::wstring sharedMemoryContent(sharedMemory);
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);

    return ParsePhoneBookData(sharedMemoryContent);
}

// Функция поиска записей по номеру телефона
std::vector<PhoneBookEntry> SearchByPhone(const std::wstring& phone, const std::vector<PhoneBookEntry>& phonebookData) {
    std::vector<PhoneBookEntry> results;

    // Проходим по всем записям и ищем совпадения по номеру (подстрока)
    for (const auto& record : phonebookData) {
        if (record.phone.find(phone) != std::wstring::npos) { // Проверяем, содержится ли phone в record.phone
            results.push_back(record); // Добавляем в результаты, если найдено совпадение
        }
    }

    // Возвращаем найденные записи (массив может быть пустым, если ничего не найдено)
    return results;
}

// Функция для получения списка всех номеров
std::vector<PhoneBookEntry> GetPhoneList(const std::vector<PhoneBookEntry>& phonebookData) {
    std::vector<PhoneBookEntry> results;

    // Проходим по всем записям
    for (const auto& record : phonebookData) {
        results.push_back(record); // Добавляем в результаты
    }

    // Возвращаем найденные записи (массив может быть пустым, если ничего не найдено)
    return results;
}
