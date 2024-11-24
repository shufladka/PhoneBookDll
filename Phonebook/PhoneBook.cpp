#include "framework.h"
#include "PhoneBook.h"
#include "PhoneBookLibrary.h"
#include <fstream>
#include <commctrl.h> // Подключаем библиотеку для работы с ListView

// Структура для хранения информации о пользователе
struct PhoneBookEntry {
    std::wstring phone;
    std::wstring lastName;
    std::wstring firstName;
    std::wstring patronymic;
    std::wstring street;
    std::wstring house;
    std::wstring building;
    std::wstring apartment;
};

#define MAX_LOADSTRING 100
#define IDC_LISTVIEW 101 // Идентификатор для ListView

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hwndListView;                              // Переменная для ListView

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL                LoadPhoneBookData(HWND hwndListView);
void                AddColumns(HWND hwndLV);
bool                ReadPhoneBookData(const std::wstring& filename, std::vector<PhoneBookEntry>& entries);
void                AddItem(HWND hwndLV, int index, const std::wstring& phone, const std::wstring& lastName,
                    const std::wstring& firstName, const std::wstring& middleName, const std::wstring& street,
                    const std::wstring& house, const std::wstring& building, const std::wstring& apartment);
//void                AddItem(HWND hwndLV, int index, LPCWSTR phone, LPCWSTR lastName, LPCWSTR firstName, LPCWSTR patronymic, LPCWSTR street, LPCWSTR house, LPCWSTR building, LPCWSTR apartment);
void                ResizeListView(HWND hwnd, int width, int height);


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

bool FileExists(const std::wstring& filename)
{
    std::wifstream file(filename);
    return file.is_open();
}

// Функция для чтения данных из файла
bool ReadPhoneBookData(const std::wstring& filename, std::vector<PhoneBookEntry>& entries) {
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

    file.close();
    return true;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

    HWND hWnd = CreateWindowW(
        szWindowClass,               // имя класса
        szTitle,                     // заголовок окна
        WS_OVERLAPPEDWINDOW,         // стиль окна
        CW_USEDEFAULT, CW_USEDEFAULT,// начальная позиция
        850, 600,                    // начальные размеры окна (ширина и высота)
        nullptr,                     // родительское окно
        nullptr,                     // меню
        hInstance,                   // экземпляр
        nullptr                      // дополнительные данные
    );


    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Создание ListView
    hwndListView = CreateWindowW(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT,
        10, 10, 800, 600, hWnd, (HMENU)IDC_LISTVIEW, hInstance, nullptr);

    // Добавление колонок в ListView
    AddColumns(hwndListView);
    
    BOOL instance = LoadPhoneBookData(hwndListView);
    return instance;
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
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        // Перемещаем и изменяем размер ListView при изменении размера окна
        ResizeListView(hWnd, width - 20, height - 20); // Учитываем отступы
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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

    // Добавляем колонки для таблицы
    const WCHAR* columns[] = { L"Телефон", L"Фамилия", L"Имя", L"Отчество", L"Улица", L"Дом", L"Корпус", L"Квартира" };
    int columnCount = sizeof(columns) / sizeof(columns[0]);

    for (int i = 0; i < columnCount; i++)
    {
        lvColumn.pszText = (WCHAR*)columns[i];
        lvColumn.cx = 100;  // Ширина колонки
        ListView_InsertColumn(hwndLV, i, &lvColumn);
    }
}

void AddItem(HWND hwndLV, int index, const std::wstring& phone, const std::wstring& lastName,
    const std::wstring& firstName, const std::wstring& middleName, const std::wstring& street,
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
    ListView_SetItemText(hwndLV, index, 3, const_cast<LPWSTR>(middleName.c_str()));
    ListView_SetItemText(hwndLV, index, 4, const_cast<LPWSTR>(street.c_str()));
    ListView_SetItemText(hwndLV, index, 5, const_cast<LPWSTR>(house.c_str()));
    ListView_SetItemText(hwndLV, index, 6, const_cast<LPWSTR>(building.c_str()));
    ListView_SetItemText(hwndLV, index, 7, const_cast<LPWSTR>(apartment.c_str()));
}

// Инициализация данных и их добавление в ListView
BOOL LoadPhoneBookData(HWND hwndListView)
{
    // Путь к файлу
    std::wstring filePath = L"E:/MVSLibrary/PhoneBookDll/x64/Debug/phonebook_db.txt";

    // Проверка существования файла
    if (!FileExists(filePath))
    {
        MessageBoxW(hwndListView, L"Файл phonebook_db.txt не найден или не доступен", L"Ошибка", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // Чтение данных из phonebook.txt и добавление строк в ListView
    std::vector<PhoneBookEntry> phonebookData;
    if (ReadPhoneBookData(filePath, phonebookData))
    {

        for (size_t i = 0; i < phonebookData.size(); i++)
        {
            const auto& entry = phonebookData[i];  // Получаем объект PhoneBookEntry

            // Передаем данные в AddItem, используя поля структуры
            AddItem(hwndListView, i, entry.phone.c_str(), entry.lastName.c_str(), entry.firstName.c_str(),
                entry.patronymic.c_str(), entry.street.c_str(), entry.house.c_str(), entry.building.c_str(),
                entry.apartment.c_str());
        }
    }

    return TRUE;
}

void ResizeListView(HWND hwnd, int width, int height)
{
    // Устанавливаем новые размеры для ListView
    MoveWindow(hwndListView, 10, 10, width, height, TRUE);
}
