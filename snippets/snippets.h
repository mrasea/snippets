#pragma once

#include "framework.h"
#include "resource.h"
#pragma comment(lib, "Comctl32.lib") // ���� Comctl32.lib ���ļ�

#define WIN_W 220
#define WIN_H 300


struct SNPI {
    std::string hLabel;
    std::string hValue;
};


ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void AddTrayIcon(HWND);
void PopupTray(HWND hwnd, LPARAM lParam);
HWND GetForegroundWindowFocus();
HWND SelectStrToFocus(POINT pt);
void ParseCsvAndStore();
LPARAM StringToLPARAM(const std::string& str);
TCHAR* StringToTCHAR(const std::string& str);
std::string TCHARToString(const TCHAR* tcharString);
std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void HideWindowAndPaste(UINT uElapse);
void OutputDebugPrintf(const char* strOutputString, ...);
int SendSelectValueToWindows();