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
        wstringstream ss;
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

    // Очистить общую память перед записью новых данных
    memset(sharedMemory, 0, fileSize);  // Очистка памяти перед записью новых данных

    // Записываем данные в общую память
    wifstream file(unicodeFilename);
    file.imbue(locale(".1251")); // Для ANSI (Windows-1251)
    wstring fileContent;
    wstring line;
    while (getline(file, line)) {
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
        // Зануляем содержимое памяти
        memset(sharedMemory, 0, fileSize);

        // Удаляем отображение памяти
        UnmapViewOfFile(sharedMemory);
        sharedMemory = NULL;
    }

    if (hMapping != NULL) {
        // Закрываем объект проецирования
        CloseHandle(hMapping);
        hMapping = NULL;
    }

    // Полностью удаляем объект проецирования
    HANDLE hTempMapping = OpenFileMappingW(FILE_MAP_WRITE, FALSE, sharedMemoryName);
    if (hTempMapping != NULL) {
        CloseHandle(hTempMapping);
        DeleteFileW(sharedMemoryName); // Удаление файла, если он связан с объектом
    }

    // Сбрасываем метаданные
    fileSize = 0;
}

// Проверка на наличие данных в общей памяти
bool IsSharedMemoryEmpty(HWND hwnd) {

    // Подключаемся к существующему объекту общей памяти
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(hwnd, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
        return true; // Если памяти нет, считаем её "пустой"
    }

    // Мапим память в адресное пространство процесса
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(hwnd, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return true; // Если не удалось отобразить, считаем её пустой
    }

    // Проверяем, есть ли данные (проверка на пустую строку)
    bool isEmpty = (sharedMemory[0] == L'\0');

    // Закрываем ресурсы
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);

    return isEmpty;
}

// Десериализация записей справочника в массив сущностей
vector<PhoneBookEntry> ParsePhoneBookData(const wstring& sharedMemoryContent) {
    vector<PhoneBookEntry> phonebookData;
    wistringstream stream(sharedMemoryContent);
    wstring line;

    while (getline(stream, line)) {
        if (line.empty()) continue; // Пропускаем пустые строки

        wstringstream ss(line);
        wstring phone, lastName, firstName, patronymic, street, house, building, apartment;

        // Разделяем строку по символу ';'
        getline(ss, phone, L';');
        getline(ss, lastName, L';');
        getline(ss, firstName, L';');
        getline(ss, patronymic, L';');
        getline(ss, street, L';');
        getline(ss, house, L';');
        getline(ss, building, L';');
        getline(ss, apartment, L';');

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
vector<PhoneBookEntry> LoadDatabaseFromMemory(HWND hwndListView) {
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

    wstring sharedMemoryContent(sharedMemory);
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);

    return ParsePhoneBookData(sharedMemoryContent);
}

// Функция поиска записей по номеру телефона
vector<PhoneBookEntry> SearchByPhone(const wstring& phone, const vector<PhoneBookEntry>& phonebookData) {
    vector<PhoneBookEntry> results;

    // Проходим по всем записям и ищем совпадения по номеру (подстрока)
    for (const auto& record : phonebookData) {
        if (record.phone.find(phone) != wstring::npos) { // Проверяем, содержится ли phone в record.phone
            results.push_back(record); // Добавляем в результаты, если найдено совпадение
        }
    }

    // Возвращаем найденные записи (массив может быть пустым, если ничего не найдено)
    return results;
}

// Функция для получения списка всех номеров
vector<PhoneBookEntry> GetPhoneList(const vector<PhoneBookEntry>& phonebookData) {
    vector<PhoneBookEntry> results;

    // Проходим по всем записям
    for (const auto& record : phonebookData) {
        results.push_back(record); // Добавляем в результаты
    }

    // Возвращаем найденные записи (массив может быть пустым, если ничего не найдено)
    return results;
}
