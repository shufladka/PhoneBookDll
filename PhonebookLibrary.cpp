#include "pch.h"
#include "PhonebookLibrary.h"

// Функция для чтения данных из файла
bool LoadDatabase(const WCHAR* filename, std::vector<PhoneBookEntry>& entries) {

    // Открываем файл для чтения
    hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Получаем размер файла
    fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return false;
    }

    // Создаём отображение файла
    hMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapping) {
        CloseHandle(hFile);
        return false;
    }

    // Мапим файл в память
    fileData = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!fileData) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }

    std::wifstream file(filename);  // Открываем файл для чтения
    if (!file.is_open()) {
        return false;  // Если не удалось открыть файл
    }

    std::wstring line;
    while (std::getline(file, line)) {
        std::wstringstream ss(line);  // Создаём строковый поток для разбора строки
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

        // Добавляем данные в список
        PhoneBookEntry entry = { phone, lastName, firstName, patronymic, street, house, building, apartment };
        entries.push_back(entry);
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
