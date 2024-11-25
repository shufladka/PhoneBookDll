#pragma once

#include "resource.h"
#include "PhoneBookLibrary.h"
#include <fstream>
#include <commctrl.h>

#define MAX_LOADSTRING 100
#define IDC_LISTVIEW 101 // ������������� ��� ListView

#define OnMenuClicked1	1
#define OnMenuClicked2	2
#define OnMenuClicked3	3
#define OnExitProgramm	0
#define OnClearedField	5
#define OnReadFile		6

HWND hEditControl;

#define TextBufferSize	1000

// ���������� ����������:
HINSTANCE hInst;                                // ������� ���������
WCHAR szTitle[MAX_LOADSTRING];                  // ����� ������ ���������
WCHAR szWindowClass[MAX_LOADSTRING];            // ��� ������ �������� ����
HWND hwndListView;                              // ���������� ��� ListView

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL                LoadPhoneBookData(HWND hwndListView);
void                AddColumns(HWND hwndLV);
void                AddItem(HWND hwndLV, int index, const std::wstring& phone, const std::wstring& lastName,
                    const std::wstring& firstName, const std::wstring& middleName, const std::wstring& street,
                    const std::wstring& house, const std::wstring& building, const std::wstring& apartment);
void                ResizeListView(HWND hwnd, int width, int height);
void MainWndAddWidgets(HWND hwnd);