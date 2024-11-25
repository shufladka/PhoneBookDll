#pragma once

#include "resource.h"
#include "PhoneBookLibrary.h"
#include <fstream>
#include <commctrl.h>
#include <commdlg.h>

#define MAX_LOADSTRING 100
#define IDC_LISTVIEW 101 // ������������� ��� ListView

#define OnExitProgramm	0
#define OnClearedField	1
#define OnSearch    	2
#define OnReadFile		3

HWND hEditControl;
std::vector<PhoneBookEntry> phonebookData;

#define TextBufferSize	1000

char filename[MAX_PATH]; // ��� �������� ���� � ���������� �����
OPENFILENAMEA ofn;        // ��������� ��� ������� �������� �����

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
void MainWndAddMenues(HWND hwnd);
void MainWndAddWidgets(HWND hwnd);
void ClearListView(HWND hwndListView);
void OnSearchByPhone(HWND hEditControl, HWND hListView);
void SetOpenFileParams(HWND hwnd);
void LoadDataFromMenu(HWND hwndListView, HWND hwndOwner);
void ConvertToUnicode(const char* ansiStr, WCHAR* unicodeStr, size_t unicodeStrSize);
