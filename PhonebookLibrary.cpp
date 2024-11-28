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

/*
// Выгрузка данных из текстового файла в общую память
BOOL UploadToDatabase(HWND hwndListView, const char* filename) {
    // Освобождение старых данных в общей памяти перед загрузкой новых
    CleanupResources(); // Функция для очистки старых данных

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

    // Очистить общую память перед записью новых данных
    memset(sharedMemory, 0, fileSize);  // Очистка памяти перед записью новых данных

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

    // Очистить ListView перед загрузкой новых данных
    //ListView_DeleteAllItems(hwndListView); // Очищаем ListView от старых данных

    // Загрузить новые данные в ListView (вам нужно будет адаптировать этот код)
    // Например, если новые данные находятся в `sharedMemory`, вы можете использовать функцию
    // для их добавления в ListView.
    //LoadDataIntoListView(hwndListView, sharedMemory);

    MessageBoxW(hwndListView, L"Данные успешно загружены в общую память!", L"Информация", MB_OK);
    return TRUE;
}
*/

//BOOL UploadToDatabase(HWND hwndListView, const char* filename) {
//    // Очистка существующей общей памяти
//    ClearSharedMemory();
//
//    // Далее ваш исходный код для загрузки файла:
//    HANDLE hFile = INVALID_HANDLE_VALUE;
//    HANDLE hMapping = NULL;
//    wchar_t* sharedMemory = NULL;
//
//    // Преобразование пути к файлу и открытие файла
//    WCHAR unicodeFilename[MAX_PATH] = { 0 };
//    ConvertToUnicode(filename, unicodeFilename, sizeof(unicodeFilename) / sizeof(WCHAR));
//
//    hFile = CreateFileW(unicodeFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//    if (hFile == INVALID_HANDLE_VALUE) {
//        MessageBoxW(hwndListView, L"Не удалось открыть файл!", L"Ошибка", MB_OK | MB_ICONERROR);
//        return FALSE;
//    }
//
//    DWORD fileSize = GetFileSize(hFile, NULL);
//    if (fileSize == 0 || fileSize == INVALID_FILE_SIZE) {
//        MessageBoxW(hwndListView, L"Размер файла некорректен или равен нулю!", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(hFile);
//        return FALSE;
//    }
//
//    hMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, fileSize, sharedMemoryName);
//    if (hMapping == NULL) {
//        MessageBoxW(hwndListView, L"Ошибка создания объекта проецирования файла!", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(hFile);
//        return FALSE;
//    }
//
//    sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, fileSize);
//    if (sharedMemory == NULL) {
//        MessageBoxW(hwndListView, L"Не удалось отобразить файл в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(hMapping);
//        CloseHandle(hFile);
//        return FALSE;
//    }
//
//    // Логика загрузки данных в память
//    std::wifstream file(unicodeFilename);
//    file.imbue(std::locale(".1251")); // Для ANSI
//    std::wstring fileContent;
//    std::wstring line;
//    while (std::getline(file, line)) {
//        fileContent += line + L"\n";
//    }
//    memcpy(sharedMemory, fileContent.c_str(), fileContent.size() * sizeof(wchar_t));
//
//    UnmapViewOfFile(sharedMemory);
//    CloseHandle(hMapping);
//    CloseHandle(hFile);
//
//    MessageBoxW(hwndListView, L"Данные успешно загружены в общую память!", L"Информация", MB_OK);
//    return TRUE;
//}

void OverwriteSharedMemoryWithEmptyValue() {
    // Подключаемся к существующему объекту общей памяти
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_WRITE, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(NULL, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Отображаем память в адресное пространство процесса
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(NULL, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }

    // Записываем пустое значение (нулевой символ) в память
    sharedMemory[0] = L'\0';

    // Закрываем отображение памяти
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);

    MessageBoxW(NULL, L"Общая память успешно очищена!", L"Информация", MB_OK);
}


// Выгрузка данных из текстового файла в общую память
BOOL UploadToDatabase(HWND hwndListView, const char* filename) {
    // Освобождение старых данных в общей памяти перед загрузкой новых
    OverwriteSharedMemoryWithEmptyValue(); // Функция для очистки старых данных

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

    // Очистить общую память перед записью новых данных
    memset(sharedMemory, 0, fileSize);  // Очистка памяти перед записью новых данных

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
    fileLoaded = false;
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
std::vector<PhoneBookEntry> LoadDatabaseFromMemory(HWND hwndListView) {
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
