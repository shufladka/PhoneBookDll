#pragma once

#include "resource.h"
#include "PhoneBookLibrary.h"
#include <fstream>
#include <commctrl.h>

// Структура для хранения информации о пользователе
//struct PhoneBookEntry {
//    std::wstring phone;
//    std::wstring lastName;
//    std::wstring firstName;
//    std::wstring patronymic;
//    std::wstring street;
//    std::wstring house;
//    std::wstring building;
//    std::wstring apartment;
//};

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
//bool                ReadPhoneBookData(const std::wstring& filename, std::vector<PhoneBookEntry>& entries);
void                AddItem(HWND hwndLV, int index, const std::wstring& phone, const std::wstring& lastName,
    const std::wstring& firstName, const std::wstring& middleName, const std::wstring& street,
    const std::wstring& house, const std::wstring& building, const std::wstring& apartment);
//void                AddItem(HWND hwndLV, int index, LPCWSTR phone, LPCWSTR lastName, LPCWSTR firstName, LPCWSTR patronymic, LPCWSTR street, LPCWSTR house, LPCWSTR building, LPCWSTR apartment);
void                ResizeListView(HWND hwnd, int width, int height);