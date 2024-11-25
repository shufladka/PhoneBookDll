#include "framework.h"
#include "PhoneBook.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PHONEBOOK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Инициализация библиотеки Common Controls
    InitCommonControls();  // Инициализируем общие элементы управления

    // Выполнить инициализацию приложения:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PHONEBOOK));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PHONEBOOK));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PHONEBOOK);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

    /*
    // Шаг 1: Создание меню
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();

    // Добавляем пункты меню
    AppendMenuW(hSubMenu, MF_STRING, OnClearedField, L"&Очистить");
    //AppendMenuW(hSubMenu, MF_STRING, ID_FILE_EXIT, L"E&xit");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"&File");
    */

    HWND hWnd = CreateWindowW(
        szWindowClass,               // имя класса
        szTitle,                     // заголовок окна
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // стиль окна
        (GetSystemMetrics(SM_CXSCREEN) - 800) / 2, // Центрирование по горизонтали
        (GetSystemMetrics(SM_CYSCREEN) - 500) / 2, // Центрирование по вертикали
        815,                       // ширина окна
        500,                       // высота окна
        nullptr,                   // родительское окно
        nullptr,                   // меню
        hInstance,                 // экземпляр
        nullptr                    // дополнительные данные
    );

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Создание ListView
    hwndListView = CreateWindowW(
        WC_LISTVIEW,
        L"",
        WS_BORDER | WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_VSCROLL | WS_HSCROLL,
        0,
        60,
        800,
        381,
        hWnd,
        (HMENU)IDC_LISTVIEW,
        hInstance,
        nullptr
    );


    // Добавление колонок в ListView
    AddColumns(hwndListView);
    
    //BOOL instance = LoadPhoneBookData(hwndListView);
    //return instance;
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Разобрать выбор в меню:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case OnClearedField:
            SetWindowTextA(hEditControl, "");
            break;
        case OnReadFile:
            LoadDataFromMenu(hwndListView, hWnd);
            //LoadPhoneBookData(hwndListView);
            break;
        case OnLoadDatabase:
            //LoadDataFromSharedMemory(hwndListView);
            //LoadDataFromSharedMemory();
            //ShowMemoryContents(hwndListView);  // hwndListView - это дескриптор окна, в котором будет показываться сообщение
            LoadPhoneBookDataToListViewAndVector(hwndListView, phonebookData);

            break;
        case OnSearch:
            OnSearchByPhone(hEditControl, hwndListView);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_CREATE: 
        InitializeSharedMemory(hWnd);
        MainWndAddMenues(hWnd);
        MainWndAddWidgets(hWnd);
        break;
    case WM_DESTROY:
        CleanupSharedMemory();
        UnloadDatabase();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/*
void InitializeSharedMemory(HWND hWnd) {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";
    bool isFirstInstance = false;

    // Попробуем открыть существующее отображение памяти
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        // Если отображение памяти не существует, создаём его
        hMapping = CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            MAX_MEMORY_SIZE, // Максимальный размер памяти
            sharedMemoryName
        );

        if (hMapping == NULL) {
            MessageBoxW(hWnd, L"Не удалось создать общую память!", L"Ошибка", MB_OK | MB_ICONERROR);
            return;
        }

        isFirstInstance = true;
    }

    // Мапим память в адресное пространство процесса
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(
        hMapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        0
    );

    if (sharedMemory == NULL) {
        MessageBoxW(hWnd, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }
    */

    // Если это первый экземпляр, загружаем данные из файла в память
    //if (isFirstInstance) {
    //    std::wstring filePath = L"E:/MVSLibrary/PhoneBookDll/x64/Debug/phonebook_db.txt";
    //    std::wifstream file(filePath);

    //    if (!file.is_open()) {
    //        MessageBoxW(hWnd, L"Не удалось открыть файл базы данных!", L"Ошибка", MB_OK | MB_ICONERROR);
    //    }
    //    else {
    //        // Считываем содержимое файла в разделяемую память
    //        std::wstring fileData((std::istreambuf_iterator<wchar_t>(file)),
    //            std::istreambuf_iterator<wchar_t>());
    //        wcscpy_s(sharedMemory, MAX_MEMORY_SIZE / sizeof(wchar_t), fileData.c_str());
    //    }
    //}
//}

/*
void LoadDataFromSharedMemory(HWND hwndListView) {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";

    // Подключаемся к существующему файлу отображения памяти
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(hwndListView, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Мапим память в адресное пространство процесса
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(
        hMapping,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (sharedMemory == NULL) {
        MessageBoxW(hwndListView, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }

    // Разбираем данные из памяти и добавляем их в ListView
    std::wstringstream dataStream(sharedMemory);
    std::wstring line;
    while (std::getline(dataStream, line)) {
        std::wstringstream ss(line);
        std::wstring phone, lastName, firstName, patronymic, street, house, building, apartment;

        std::getline(ss, phone, L';');
        std::getline(ss, lastName, L';');
        std::getline(ss, firstName, L';');
        std::getline(ss, patronymic, L';');
        std::getline(ss, street, L';');
        std::getline(ss, house, L';');
        std::getline(ss, building, L';');
        std::getline(ss, apartment, L';');

        PhoneBookEntry entry = { phone, lastName, firstName, patronymic, street, house, building, apartment };
        AddItem(hwndListView, phonebookData.size(), entry.phone.c_str(), entry.lastName.c_str(),
            entry.firstName.c_str(), entry.patronymic.c_str(), entry.street.c_str(),
            entry.house.c_str(), entry.building.c_str(), entry.apartment.c_str());
    }

    // Закрываем отображение памяти
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);
}
*/

// Функция для создания общей памяти
bool CreateSharedMemory(const WCHAR* sharedMemoryName, size_t size, wchar_t** sharedMemory) {
    // Создаем объект общей памяти
    HANDLE hMapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,    // Используем анонимный файл
        NULL,                    // Атрибуты безопасности
        PAGE_READWRITE,          // Права доступа
        0,                       // Старший 32 бита размера
        size,                    // Младший 32 бита размера
        sharedMemoryName         // Имя объекта общей памяти
    );

    if (hMapping == NULL) {
        return false; // Не удалось создать объект общей памяти
    }

    // Мапим память в адресное пространство
    *sharedMemory = (wchar_t*)MapViewOfFile(
        hMapping,
        FILE_MAP_READ | FILE_MAP_WRITE, // Права доступа
        0,
        0,
        0
    );

    if (*sharedMemory == NULL) {
        CloseHandle(hMapping);
        return false; // Не удалось мапировать память
    }

    return true;
}

// Функция для подключения к существующему объекту общей памяти
bool ConnectToSharedMemory(const WCHAR* sharedMemoryName, wchar_t** sharedMemory) {
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        return false; // Не удалось подключиться
    }

    *sharedMemory = (wchar_t*)MapViewOfFile(
        hMapping,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    CloseHandle(hMapping); // Закрываем хендл объекта, так как мы уже мапили память

    return *sharedMemory != NULL; // Успех, если память была мапирована
}

/*
void LoadDataFromSharedMemory(HWND hwndListView) {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";

    // Подключаемся к существующему объекту разделяемой памяти
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(hwndListView, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }
    else {
        MessageBoxW(hwndListView, L"Удалось подключиться к общей памяти!", L"Не ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Мапим память в адресное пространство процесса
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(
        hMapping,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (sharedMemory == NULL) {
        MessageBoxW(hwndListView, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }

    // Читаем данные из памяти
    ReadFromSharedMemory(sharedMemory, hwndListView);

    // Закрываем отображение памяти
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);
}
*/
/*
// Функция загрузки данных из общей памяти
void LoadDataFromSharedMemory(HWND hwndListView) {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";

    // Для первого экземпляра программы создаём общую память, если её ещё нет
    wchar_t* sharedMemory = nullptr;
    if (!ConnectToSharedMemory(sharedMemoryName, &sharedMemory)) {
        if (!CreateSharedMemory(sharedMemoryName, MAX_MEMORY_SIZE, &sharedMemory)) {
            MessageBoxW(hwndListView, L"Не удалось подключиться или создать общую память!", L"Ошибка", MB_OK | MB_ICONERROR);
            return;
        }
    }

    // Если память успешно подключена, читаем данные
    if (sharedMemory) {
        MessageBoxW(hwndListView, L"Удалось подключиться к общей памяти!", L"Информация", MB_OK);
        // Чтение данных
        ReadFromSharedMemory(sharedMemory, hwndListView);
    }
    else {
        MessageBoxW(hwndListView, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
    }

    // Закрытие общего доступа
    //UnmapViewOfFile(sharedMemory);
}
*/
void LoadDataFromSharedMemory() {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";

    // Подключаемся к существующему объекту общей памяти
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(NULL, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Мапим память в адресное пространство
    void* sharedMemory = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(NULL, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }

    // Читаем данные из общей памяти
    PhoneBookEntry* entries = (PhoneBookEntry*)sharedMemory;
    for (int i = 0; i < MAX_RECORDS; ++i) {
        // Здесь вы можете добавить код для вывода или обработки записей
        wprintf(L"Phone: %s\n", entries[i].phone.c_str());
    }

    // Закрываем все
    //UnmapViewOfFile(sharedMemory);
    //CloseHandle(hMapping);
}



void CleanupSharedMemory() {
    if (sharedMemory != NULL) {
        UnmapViewOfFile(sharedMemory);
    }

    if (hMapping != NULL) {
        CloseHandle(hMapping);
    }
}

void InitializeSharedMemory(HWND hWnd) {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";
    bool isFirstInstance = false;

    // Создаём или открываем объект отображения памяти
    HANDLE hMapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,  // Используем системную память
        NULL,                  // Без защиты
        PAGE_READWRITE,        // Память доступна для чтения и записи
        0,                     // Высокие 32 бита размера (0, т.к. используем MAX_MEMORY_SIZE)
        MAX_MEMORY_SIZE,       // Низкие 32 бита размера
        sharedMemoryName       // Имя объекта
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

    // Если это первый экземпляр, загружаем данные в память
    /*if (isFirstInstance) {
        std::wstring filePath = L"E:/MVSLibrary/PhoneBookDll/x64/Debug/phonebook_db.txt";
        if (!WriteToSharedMemory(filePath, sharedMemory, MAX_MEMORY_SIZE)) {
            MessageBoxW(hWnd, L"Не удалось записать данные в общую память!", L"Ошибка", MB_OK | MB_ICONERROR);
        }
        else {
            MessageBoxW(hWnd, L"Удалось записать данные в общую память!", L"Не ошибка", MB_OK | MB_ICONERROR);
        }
    }*/

    // Закрываем объект отображения памяти (но разделяемая память остаётся доступной)
    //UnmapViewOfFile(sharedMemory);
    //CloseHandle(hMapping);
}


/*
void ReadFromSharedMemory(wchar_t* sharedMemory, HWND hwndListView) {
    if (sharedMemory == nullptr) {
        MessageBoxW(hwndListView, L"Ошибка: общая память пуста.", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Пример вывода первого блока данных из общей памяти
    MessageBoxW(hwndListView, sharedMemory, L"Данные из общей памяти", MB_OK);

    // Понимание структуры данных, прочтите их по частям
    std::wstring data(sharedMemory);
    MessageBoxW(hwndListView, data.c_str(), L"Чтение из памяти", MB_OK);

    std::wstringstream dataStream(sharedMemory);
    std::wstring line;

    while (std::getline(dataStream, line)) {
        std::wstringstream ss(line);
        std::wstring phone, lastName, firstName, patronymic, street, house, building, apartment;

        // Разделяем строку на части
        std::getline(ss, phone, L';');
        std::getline(ss, lastName, L';');
        std::getline(ss, firstName, L';');
        std::getline(ss, patronymic, L';');
        std::getline(ss, street, L';');
        std::getline(ss, house, L';');
        std::getline(ss, building, L';');
        std::getline(ss, apartment, L';');

        // Добавляем данные в ListView
        AddItem(hwndListView, phonebookData.size(), phone.c_str(), lastName.c_str(),
            firstName.c_str(), patronymic.c_str(), street.c_str(), house.c_str(),
            building.c_str(), apartment.c_str());
    }
}
*/

void ReadFromSharedMemory(wchar_t* sharedMemory, HWND hwndListView) {
    if (sharedMemory == nullptr) {
        MessageBoxW(hwndListView, L"Ошибка: общая память пуста.", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Прочитаем данные и выведем в MessageBox
    size_t index = 0;
    std::wstring phone, lastName, firstName, patronymic, street, house, building, apartment;
    for (size_t i = 0; i < phonebookData.size(); ++i) {
        phone = sharedMemory + index;
        index += MAX_ENTRY_SIZE;

        lastName = sharedMemory + index;
        index += MAX_ENTRY_SIZE;

        firstName = sharedMemory + index;
        index += MAX_ENTRY_SIZE;

        patronymic = sharedMemory + index;
        index += MAX_ENTRY_SIZE;

        street = sharedMemory + index;
        index += MAX_ENTRY_SIZE;

        house = sharedMemory + index;
        index += MAX_ENTRY_SIZE;

        building = sharedMemory + index;
        index += MAX_ENTRY_SIZE;

        apartment = sharedMemory + index;
        index += MAX_ENTRY_SIZE;

        MessageBoxW(hwndListView, phone.c_str(), L"Phone", MB_OK);
        MessageBoxW(hwndListView, lastName.c_str(), L"Last Name", MB_OK);
        // Выведите другие поля аналогично
    }
}





INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void AddColumns(HWND hwndLV)
{
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    // Описание колонок: текст и ширина
    struct ColumnInfo {
        int width;         // Ширина колонки
        const WCHAR* name; // Имя колонки
    };

    ColumnInfo columns[] = {
        { 100, L"Телефон" },
        { 100, L"Фамилия" },
        { 100, L"Имя" },
        { 100, L"Отчество" },
        { 100, L"Улица", },
        { 100, L"Дом" },
        { 100, L"Корпус" },
        { 100, L"Квартира" }
    };

    int columnCount = sizeof(columns) / sizeof(columns[0]);

    // Добавление колонок в ListView
    for (int i = 0; i < columnCount; ++i)
    {
        lvColumn.pszText = const_cast<LPWSTR>(columns[i].name);
        lvColumn.cx = columns[i].width;
        ListView_InsertColumn(hwndLV, i, &lvColumn);
    }
}


void AddItem(HWND hwndLV, int index, const std::wstring& phone, const std::wstring& lastName,
    const std::wstring& firstName, const std::wstring& patronymic, const std::wstring& street,
    const std::wstring& house, const std::wstring& building, const std::wstring& apartment)
{
    LVITEM lvItem;
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = index;
    lvItem.iSubItem = 0;
    lvItem.pszText = const_cast<LPWSTR>(phone.c_str());  // Конвертируем std::wstring в LPWSTR
    ListView_InsertItem(hwndLV, &lvItem);

    // Добавление данных в другие колонки
    ListView_SetItemText(hwndLV, index, 1, const_cast<LPWSTR>(lastName.c_str()));
    ListView_SetItemText(hwndLV, index, 2, const_cast<LPWSTR>(firstName.c_str()));
    ListView_SetItemText(hwndLV, index, 3, const_cast<LPWSTR>(patronymic.c_str()));
    ListView_SetItemText(hwndLV, index, 4, const_cast<LPWSTR>(street.c_str()));
    ListView_SetItemText(hwndLV, index, 5, const_cast<LPWSTR>(house.c_str()));
    ListView_SetItemText(hwndLV, index, 6, const_cast<LPWSTR>(building.c_str()));
    ListView_SetItemText(hwndLV, index, 7, const_cast<LPWSTR>(apartment.c_str()));
}

void ShowMemoryContents(HWND hwnd) {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";

    // Подключаемся к существующему объекту общей памяти
    HANDLE hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(hwnd, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Мапим память в адресное пространство процесса
    wchar_t* sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(hwnd, L"Не удалось отобразить память в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return;
    }

    // Создаем строку с содержимым общей памяти
    std::wstring memoryContents(sharedMemory);

    // Отображаем содержимое в MessageBox
    MessageBoxW(hwnd, memoryContents.c_str(), L"Содержимое общей памяти", MB_OK);

    // Закрываем отображение памяти
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);
}


// Инициализация данных и их добавление в ListView
//BOOL LoadPhoneBookData(HWND hwndListView)
//{
//    // Путь к файлу
//    const WCHAR* filePath = L"E:/MVSLibrary/PhoneBookDll/x64/Debug/phonebook_db.txt";
//
//    // Чтение данных из phonebook.txt и добавление строк в ListView
//    if (LoadDatabase(filePath, phonebookData))
//    {
//
//        for (size_t i = 0; i < phonebookData.size(); i++)
//        {
//            const auto& entry = phonebookData[i];  // Получаем объект PhoneBookEntry
//
//            // Передаем данные в AddItem, используя поля структуры
//            AddItem(hwndListView, i, entry.phone.c_str(), entry.lastName.c_str(), entry.firstName.c_str(),
//                entry.patronymic.c_str(), entry.street.c_str(), entry.house.c_str(), entry.building.c_str(),
//                entry.apartment.c_str());
//        }
//    }
//
//    return TRUE;
//}

BOOL LoadPhoneBookData(HWND hwndListView) {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";
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

BOOL LoadPhoneBookDataToListViewAndVector(HWND hwndListView, std::vector<PhoneBookEntry>& phonebookData) {
    const WCHAR* sharedMemoryName = L"PhoneBookSharedMemory";
    HANDLE hMapping = NULL;
    wchar_t* sharedMemory = NULL;

    // Подключаемся к существующему объекту разделяемой памяти
    hMapping = OpenFileMappingW(FILE_MAP_READ, FALSE, sharedMemoryName);
    if (hMapping == NULL) {
        MessageBoxW(hwndListView, L"Не удалось подключиться к общей памяти!", L"Ошибка", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // Мапим файл в адресное пространство
    sharedMemory = (wchar_t*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (sharedMemory == NULL) {
        MessageBoxW(hwndListView, L"Не удалось отобразить файл в адресное пространство!", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hMapping);
        return FALSE;
    }

    // Разбираем данные из общей памяти
    std::wistringstream stream(sharedMemory);
    std::wstring line;

    int index = 0;
    phonebookData.clear(); // Очищаем вектор перед загрузкой новых данных

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

        // Добавляем запись в вектор
        phonebookData.push_back(entry);

        // Добавляем запись в ListView
        AddItem(hwndListView, index, phone.c_str(), lastName.c_str(), firstName.c_str(),
            patronymic.c_str(), street.c_str(), house.c_str(), building.c_str(), apartment.c_str());
        index++;
    }

    // Освобождаем ресурсы
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);

    MessageBoxW(hwndListView, L"Данные успешно загружены из общей памяти и записаны в вектор!", L"Информация", MB_OK);
    return TRUE;
}



/*
BOOL LoadPhoneBookData(HWND hwndListView) {
    // Преобразуем строку пути к файлу из ANSI в Unicode
    WCHAR unicodeFilename[MAX_PATH] = { 0 };
    ConvertToUnicode(filename, unicodeFilename, sizeof(unicodeFilename) / sizeof(WCHAR));

    // Теперь передаем в LoadDatabase строку в формате WCHAR
    if (LoadDatabase(unicodeFilename, phonebookData)) {
        // Проходим по всем данным и добавляем их в ListView
        for (size_t i = 0; i < phonebookData.size(); i++) {
            const auto& entry = phonebookData[i];
            AddItem(hwndListView, i, entry.phone.c_str(), entry.lastName.c_str(), entry.firstName.c_str(),
                entry.patronymic.c_str(), entry.street.c_str(), entry.house.c_str(), entry.building.c_str(),
                entry.apartment.c_str());
        }
        return TRUE;
    }
    return FALSE;
}
*/

// Преобразование строки из ANSI в Unicode
void ConvertToUnicode(const char* ansiStr, WCHAR* unicodeStr, size_t unicodeStrSize) {
    MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, unicodeStr, unicodeStrSize);
}

void ResizeListView(HWND hwnd, int width, int height)
{
    // Устанавливаем новые размеры для ListView
    MoveWindow(hwndListView, 10, 10, width, height, TRUE);
}

void MainWndAddMenues(HWND hwnd) {
    HMENU RootMenu = CreateMenu();
    HMENU SubMenu = CreateMenu();

    AppendMenu(SubMenu, MF_POPUP, OnClearedField, L"Очистить поле ввода");
    AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(SubMenu, MF_STRING, OnReadFile, L"Загрузить из файла");
    AppendMenu(SubMenu, MF_STRING, OnLoadDatabase, L"Загрузить из памяти");
    AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(SubMenu, MF_STRING, OnExitProgramm, L"Exit");

    AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"File");
    AppendMenu(RootMenu, MF_STRING, (UINT_PTR)SubMenu, L"Help");

    SetMenu(hwnd, RootMenu);
}

// Функция для инициализации структуры OPENFILENAME
void SetOpenFileParams(HWND hwnd) {
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.lpstrFile[0] = '\0'; // Начальное значение пути
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = "Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

// Функция для обработки загрузки файла
void LoadDataFromMenu(HWND hwndListView, HWND hwndOwner) {
    SetOpenFileParams(hwndOwner);
    if (GetOpenFileNameA(&ofn)) {
        if (!LoadPhoneBookData(hwndListView)) {
            MessageBox(hwndOwner, L"Failed to load data from file.", L"Error", MB_OK | MB_ICONERROR);
        }
    }
}


void MainWndAddWidgets(HWND hwnd) {
    CreateWindowA("button", "Очистить поле", WS_VISIBLE | WS_CHILD | ES_CENTER, 25, 10, 115, 20, hwnd, (HMENU)OnClearedField, NULL, NULL);
    hEditControl = CreateWindowA("edit", "", WS_BORDER | WS_VISIBLE | WS_CHILD, 155, 10, 430, 20, hwnd, NULL, NULL, NULL);
    CreateWindowA("button", "Поиск", WS_VISIBLE | WS_CHILD | ES_CENTER, 600, 10, 150, 20, hwnd, (HMENU)OnSearch, NULL, NULL);
}

void ClearListView(HWND hwndListView)
{
    // Удаляем все элементы из ListView
    ListView_DeleteAllItems(hwndListView);
}

// Функция для извлечения текста из текстового поля и поиска записи
void OnSearchByPhone(HWND hEditControl, HWND hListView)
{
    wchar_t phone[100]; // Буфер для номера телефона
    GetWindowText(hEditControl, phone, 100); // Извлечение текста из текстового поля

    // Вызов функции поиска
    auto searchResults = SearchByPhone(phone, phonebookData);

    // Очистка ListView перед добавлением результата
    ClearListView(hListView);

    if (!searchResults.empty()) {
        // Если записи найдены, добавляем их в ListView
        for (size_t i = 0; i < searchResults.size(); ++i) {
            const auto& entry = searchResults[i];
            AddItem(hListView, i, entry.phone.c_str(), entry.lastName.c_str(), entry.firstName.c_str(),
                entry.patronymic.c_str(), entry.street.c_str(), entry.house.c_str(), entry.building.c_str(),
                entry.apartment.c_str());
        }
    }
    else {

        // Если записи не найдены, показываем все записи БД
        if (wcslen(phone) == 0) {
            for (size_t i = 0; i < phonebookData.size(); i++)
            {
                const auto& entry = phonebookData[i];  // Получаем объект PhoneBookEntry

                // Передаем данные в AddItem, используя поля структуры
                AddItem(hwndListView, i, entry.phone.c_str(), entry.lastName.c_str(), entry.firstName.c_str(),
                    entry.patronymic.c_str(), entry.street.c_str(), entry.house.c_str(), entry.building.c_str(),
                    entry.apartment.c_str());
            }
        }
    }
}

