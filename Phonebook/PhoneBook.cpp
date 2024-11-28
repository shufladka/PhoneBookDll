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
        865,                       // ширина окна
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
        850,
        381,
        hWnd,
        (HMENU)IDC_LISTVIEW,
        hInstance,
        nullptr
    );

    // Добавление колонок в ListView
    DefineColumns(hwndListView);
    
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case OnClearedField:
            SetWindowTextA(hEditControl, "");
            break;
        case OnReadFile:
            PickTheFile(hwndListView, hWnd);
            break;
        case OnLoadDatabase:
            //ShowMemoryContents(hwndListView);
            LoadDataToTable(hwndListView);
            break;
        case OnClearedList:
            ClearSharedMemory();
            ListView_DeleteAllItems(hwndListView);
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
        CleanupResources();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Определение столбцов таблицы
void DefineColumns(HWND hwndLV)
{
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    // Описание колонок: текст и ширина
    struct ColumnInfo {
        int width;         // Ширина колонки
        const WCHAR* name; // Имя колонки
    };

    ColumnInfo columns[] = {
        { 50, L"N п/п"},
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

    // Добавление колонок в таблицу
    for (int i = 0; i < columnCount; ++i)
    {
        lvColumn.pszText = const_cast<LPWSTR>(columns[i].name);
        lvColumn.cx = columns[i].width;
        ListView_InsertColumn(hwndLV, i, &lvColumn);
    }
}

// Заполнение таблицы списком записей
void PhoneBookFilling(HWND hwndListView, const vector<PhoneBookEntry>& phonebookData) {

    // Удаляем все элементы перед добавлением новых
    ListView_DeleteAllItems(hwndListView);

    for (size_t i = 0; i < phonebookData.size(); ++i) {
        const auto& entry = phonebookData[i];
        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = static_cast<int>(i);
        lvItem.iSubItem = 0;

        wstring indexText = to_wstring(i + 1);
        lvItem.pszText = const_cast<LPWSTR>(indexText.c_str());
        ListView_InsertItem(hwndListView, &lvItem);

        ListView_SetItemText(hwndListView, static_cast<int>(i), 1, const_cast<LPWSTR>(entry.phone.c_str()));
        ListView_SetItemText(hwndListView, static_cast<int>(i), 2, const_cast<LPWSTR>(entry.lastName.c_str()));
        ListView_SetItemText(hwndListView, static_cast<int>(i), 3, const_cast<LPWSTR>(entry.firstName.c_str()));
        ListView_SetItemText(hwndListView, static_cast<int>(i), 4, const_cast<LPWSTR>(entry.patronymic.c_str()));
        ListView_SetItemText(hwndListView, static_cast<int>(i), 5, const_cast<LPWSTR>(entry.street.c_str()));
        ListView_SetItemText(hwndListView, static_cast<int>(i), 6, const_cast<LPWSTR>(entry.house.c_str()));
        ListView_SetItemText(hwndListView, static_cast<int>(i), 7, const_cast<LPWSTR>(entry.building.c_str()));
        ListView_SetItemText(hwndListView, static_cast<int>(i), 8, const_cast<LPWSTR>(entry.apartment.c_str()));
    }
}

// Выгрузка сущностей записей справочника в таблицу
void LoadDataToTable(HWND hwndListView) {
    phonebookData = LoadDatabaseFromMemory(hwndListView);
    if (!phonebookData.empty()) {
        PhoneBookFilling(hwndListView, phonebookData);
    }
}

// Отображение записей справочника из общей памяти на экран в виде сообщения
void ShowMemoryContents(HWND hwnd) {

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
    wstring memoryContents(sharedMemory);

    // Отображаем содержимое в MessageBox
    MessageBoxW(hwnd, memoryContents.c_str(), L"Содержимое общей памяти", MB_OK);

    // Закрываем отображение памяти
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapping);
}

// Добавление пунктов меню
void MainWndAddMenues(HWND hwnd) {
    HMENU RootMenu = CreateMenu();
    AppendMenu(RootMenu, MF_STRING, OnReadFile, L"Загрузить из файла");
    AppendMenu(RootMenu, MF_STRING, OnLoadDatabase, L"Загрузить из памяти");
    AppendMenu(RootMenu, MF_STRING, OnClearedList, L"Очистить список");
    AppendMenu(RootMenu, MF_STRING, IDM_EXIT, L"Выход");
    SetMenu(hwnd, RootMenu);
}

// Добавление виджетов в рабочую область приложения
void MainWndAddWidgets(HWND hwnd) {
    CreateWindowA("button", "Очистить поле", WS_VISIBLE | WS_CHILD | ES_CENTER, 25, 10, 115, 20, hwnd, (HMENU)OnClearedField, NULL, NULL);
    hEditControl = CreateWindowA("edit", "", WS_BORDER | WS_VISIBLE | WS_CHILD, 155, 10, 430, 20, hwnd, NULL, NULL, NULL);
    CreateWindowA("button", "Поиск", WS_VISIBLE | WS_CHILD | ES_CENTER, 600, 10, 150, 20, hwnd, (HMENU)OnSearch, NULL, NULL);
}

// Инициализация структуры OPENFILENAME
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
void PickTheFile(HWND hwndListView, HWND hwndOwner) {

    // Проверяем, пуста ли память перед загрузкой нового файла
    if (!IsSharedMemoryEmpty(hwndOwner)) {
        MessageBoxW(hwndOwner, L"Общая память не пуста. Закройте другие экземпляры программы и повторите попытку.", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    SetOpenFileParams(hwndOwner);
    if (GetOpenFileNameA(&ofn)) {
        if (!UploadToDatabase(hwndListView, filename)) {
            MessageBox(hwndOwner, L"Ошибка загрузки данных из файла.", L"Ошибка", MB_OK | MB_ICONERROR);
        }
        else {
            LoadDataToTable(hwndListView);
        }
    }
}

// Функция для извлечения текста из текстового поля и поиска записи
void OnSearchByPhone(HWND hEditControl, HWND hListView) {
    wchar_t phone[100]; // Буфер для номера телефона
    GetWindowText(hEditControl, phone, 100); // Извлечение текста из текстового поля

    // Выполняем поиск записей по номеру телефона
    auto searchResults = SearchByPhone(phone, phonebookData);

    // Очищаем ListView перед отображением новых данных
    ListView_DeleteAllItems(hListView);

    if (!searchResults.empty()) {
        // Если найдены записи, отображаем только их
        PhoneBookFilling(hListView, searchResults);
    }
    else {
        if (wcslen(phone) == 0) {
            // Если строка поиска пуста, показываем все записи
            PhoneBookFilling(hListView, phonebookData);
        }
        else {
            // Если ничего не найдено и строка поиска не пуста
            MessageBoxW(hListView, L"Записи с указанным номером телефона не найдены.", L"Поиск", MB_OK | MB_ICONINFORMATION);
        }
    }
}
