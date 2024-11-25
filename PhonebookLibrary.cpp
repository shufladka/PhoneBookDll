#include "pch.h"
#include "PhonebookLibrary.h"

// Функция для чтения данных из файла
//bool LoadDatabase(const WCHAR* filename, std::vector<PhoneBookEntry>& entries) {
//
//    // Открываем файл для чтения
//    hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//    if (hFile == INVALID_HANDLE_VALUE) {
//        return false;
//    }
//
//    // Получаем размер файла
//    fileSize = GetFileSize(hFile, NULL);
//    if (fileSize == INVALID_FILE_SIZE) {
//        CloseHandle(hFile);
//        return false;
//    }
//
//    // Создаём отображение файла
//    hMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
//    if (!hMapping) {
//        CloseHandle(hFile);
//        return false;
//    }
//
//    // Мапим файл в память
//    fileData = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
//    if (!fileData) {
//        CloseHandle(hMapping);
//        CloseHandle(hFile);
//        return false;
//    }
//
//    std::wifstream file(filename);  // Открываем файл для чтения
//    if (!file.is_open()) {
//        return false;  // Если не удалось открыть файл
//    }
//
//    std::wstring line;
//    while (std::getline(file, line)) {
//        std::wstringstream ss(line);  // Создаём строковый поток для разбора строки
//        std::wstring phone, lastName, firstName, patronymic, street, house, building, apartment;
//
//        // Разделяем строку по символу ';'
//        std::getline(ss, phone, L';');
//        std::getline(ss, lastName, L';');
//        std::getline(ss, firstName, L';');
//        std::getline(ss, patronymic, L';');
//        std::getline(ss, street, L';');
//        std::getline(ss, house, L';');
//        std::getline(ss, building, L';');
//        std::getline(ss, apartment, L';');
//
//        // Добавляем данные в список
//        PhoneBookEntry entry = { phone, lastName, firstName, patronymic, street, house, building, apartment };
//        entries.push_back(entry);
//    }
//
//    return true;
//}
bool LoadDatabase(const WCHAR* filePath, std::vector<PhoneBookEntry>& entries) {
    std::wifstream file(filePath);
    if (!file.is_open()) {
        MessageBoxW(NULL, L"Не удалось открыть файл для чтения", L"Ошибка", MB_ICONERROR);
        return false;
    }

    std::wstring line;
    while (std::getline(file, line)) {
        entries.push_back(DeserializePhoneBookEntry(line));
    }

    return true;
}

// Освобождение ресурсов
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

// Сериализация записи в строку
std::wstring SerializePhoneBookEntry(const PhoneBookEntry& entry) {
    std::wstringstream ss;
    ss << entry.phone << L";" << entry.lastName << L";" << entry.firstName << L";"
        << entry.patronymic << L";" << entry.street << L";" << entry.house << L";"
        << entry.building << L";" << entry.apartment;
    return ss.str();
}

// Десериализация строки в запись
PhoneBookEntry DeserializePhoneBookEntry(const std::wstring& line) {
    std::wstringstream ss(line);
    PhoneBookEntry entry;
    std::getline(ss, entry.phone, L';');
    std::getline(ss, entry.lastName, L';');
    std::getline(ss, entry.firstName, L';');
    std::getline(ss, entry.patronymic, L';');
    std::getline(ss, entry.street, L';');
    std::getline(ss, entry.house, L';');
    std::getline(ss, entry.building, L';');
    std::getline(ss, entry.apartment, L';');
    return entry;
}

// Инициализация общей памяти
bool InitSharedMemory() {
    hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_MEMORY_SIZE, L"PhoneBookSharedMemory");
    if (!hMapping) {
        MessageBoxW(NULL, L"Не удалось создать отображение памяти", L"Ошибка", MB_ICONERROR);
        return false;
    }

    sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, MAX_MEMORY_SIZE);
    if (!sharedMemory) {
        MessageBoxW(NULL, L"Не удалось отобразить память", L"Ошибка", MB_ICONERROR);
        CloseHandle(hMapping);
        return false;
    }

    return true;
}

// Освобождение ресурсов
void CleanupSharedMemory() {
    if (sharedMemory) {
        UnmapViewOfFile(sharedMemory);
        sharedMemory = NULL;
    }
    if (hMapping) {
        CloseHandle(hMapping);
        hMapping = NULL;
    }
}

// Запись данных в общую память
//bool WriteToSharedMemory(const std::vector<PhoneBookEntry>& entries) {
//    if (!sharedMemory) return false;
//
//    std::wstringstream ss;
//    for (const auto& entry : entries) {
//        ss << SerializePhoneBookEntry(entry) << L"\n";
//    }
//
//    std::wstring data = ss.str();
//    if (data.size() * sizeof(wchar_t) > MAX_MEMORY_SIZE) {
//        MessageBoxW(NULL, L"Недостаточно памяти для записи данных", L"Ошибка", MB_ICONERROR);
//        return false;
//    }
//
//    memcpy(sharedMemory, data.c_str(), data.size() * sizeof(wchar_t));
//    sharedMemory[data.size()] = L'\0'; // Завершающий символ
//    return true;
//}
/*
bool WriteToSharedMemory(const std::wstring& filePath, wchar_t* sharedMemory, size_t maxMemorySize) {
    std::wifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    // Читаем содержимое файла
    std::wstring fileData((std::istreambuf_iterator<wchar_t>(file)),
        std::istreambuf_iterator<wchar_t>());

    // Проверяем, помещаются ли данные в разделяемую память
    if (fileData.size() >= maxMemorySize / sizeof(wchar_t)) {
        return false;
    }

    // Копируем данные в разделяемую память
    wcscpy_s(sharedMemory, maxMemorySize / sizeof(wchar_t), fileData.c_str());
    return true;
}
*/
/*
void WriteToSharedMemory(wchar_t* sharedMemory, const std::vector<PhoneBookEntry>& phonebookData) {
    if (sharedMemory == nullptr) {
        return;
    }

    size_t index = 0;
    for (const auto& entry : phonebookData) {
        // Запись каждого поля структуры в память
        wcscpy_s(sharedMemory + index, MAX_ENTRY_SIZE, entry.phone.c_str());
        index += MAX_ENTRY_SIZE;  // Увеличиваем индекс для следующей записи

        wcscpy_s(sharedMemory + index, MAX_ENTRY_SIZE, entry.lastName.c_str());
        index += MAX_ENTRY_SIZE;

        wcscpy_s(sharedMemory + index, MAX_ENTRY_SIZE, entry.firstName.c_str());
        index += MAX_ENTRY_SIZE;

        wcscpy_s(sharedMemory + index, MAX_ENTRY_SIZE, entry.patronymic.c_str());
        index += MAX_ENTRY_SIZE;

        wcscpy_s(sharedMemory + index, MAX_ENTRY_SIZE, entry.street.c_str());
        index += MAX_ENTRY_SIZE;

        wcscpy_s(sharedMemory + index, MAX_ENTRY_SIZE, entry.house.c_str());
        index += MAX_ENTRY_SIZE;

        wcscpy_s(sharedMemory + index, MAX_ENTRY_SIZE, entry.building.c_str());
        index += MAX_ENTRY_SIZE;

        wcscpy_s(sharedMemory + index, MAX_ENTRY_SIZE, entry.apartment.c_str());
        index += MAX_ENTRY_SIZE;
    }
}
*/

void WriteToSharedMemory(const WCHAR* filename) {
    // Создаем или открываем объект общей памяти
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";
    HANDLE hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_ENTRY_SIZE * MAX_RECORDS, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(NULL, L"Не удалось создать общую память!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Мапим память в адресное пространство
    void* sharedMemory = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(NULL, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }

    // Открываем файл и считываем данные
    std::wifstream file(filename);
    if (!file.is_open()) {
        MessageBoxW(NULL, L"Не удалось открыть файл!", L"Ошибка", MB_OK | MB_ICONERROR);
        UnmapViewOfFile(sharedMemory);
        CloseHandle(hMapping);
        return;
    }

    // Читаем данные из файла и записываем в общую память
    PhoneBookEntry* entries = (PhoneBookEntry*)sharedMemory;
    int index = 0;
    std::wstring line;
    while (std::getline(file, line) && index < MAX_RECORDS) {
        std::wstringstream ss(line);
        std::getline(ss, entries[index].phone, L';');
        std::getline(ss, entries[index].lastName, L';');
        std::getline(ss, entries[index].firstName, L';');
        std::getline(ss, entries[index].patronymic, L';');
        std::getline(ss, entries[index].street, L';');
        std::getline(ss, entries[index].house, L';');
        std::getline(ss, entries[index].building, L';');
        std::getline(ss, entries[index].apartment, L';');
        index++;
    }

    // Закрываем все
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);
}



// Чтение данных из общей памяти
//std::vector<PhoneBookEntry> ReadFromSharedMemory() {
//    std::vector<PhoneBookEntry> entries;
//
//    if (!sharedMemory) return entries;
//
//    std::wstring data(sharedMemory);
//    std::wstringstream ss(data);
//    std::wstring line;
//
//    while (std::getline(ss, line)) {
//        if (!line.empty()) {
//            entries.push_back(DeserializePhoneBookEntry(line));
//        }
//    }
//
//    return entries;
//}
