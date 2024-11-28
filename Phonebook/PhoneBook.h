#pragma once

#include "resource.h"
#include "PhoneBookLibrary.h"
#include <fstream>
#include <commctrl.h>
#include <commdlg.h>

#define MAX_LOADSTRING  100
#define IDC_LISTVIEW    101

#define OnClearedField	1
#define OnClearedList	2
#define OnSearch    	3
#define OnReadFile		4
#define OnLoadDatabase  5

#define TextBufferSize	1000

HWND hEditControl;								// Поле ввода
HWND hComboBox;									// Выпадающее меню
char filename[MAX_PATH];						// Для хранения пути к выбранному файлу
OPENFILENAMEA ofn;								// Структура для диалога открытия файла
HINSTANCE hInst;                                // Текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // Имя класса главного окна
HWND hwndListView;                              // Переменная для ListView

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void                MainWndAddMenues(HWND hwnd);
void                MainWndAddWidgets(HWND hwnd);

void                SetOpenFileParams(HWND hwnd);

void                DefineColumns(HWND hwndLV);
void				PhoneBookFilling(HWND hwndListView, const vector<PhoneBookEntry>& phonebookData);

void                PickTheFile(HWND hwndListView, HWND hwndOwner);
void				LoadDataToTable(HWND hwndListView);
void                OnSearchByField(HWND hEditControl, HWND hComboBox, HWND hListView);
