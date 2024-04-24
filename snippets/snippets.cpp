// snippets.cpp : 定义应用程序的入口点。
//


#include "snippets.h"

#pragma comment(lib, "Comctl32.lib") // 链接 Comctl32.lib 库文件
#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND hWnd;
NOTIFYICONDATA nid;
HMENU hMenu;
HHOOK hHook;

HWND hForeWnd;
HWND hInFocus;

std::vector<SNPI> g_data; // 结构体数组缓存
UINT_PTR g_nTimerID = 0; // 定时器ID

HWND popEdit;
HWND popListBox;


void OutputDebugPrintf(const char* strOutputString, ...)
{
    char strBuffer[4096] = { 0 };
    va_list vlArgs;
    va_start(vlArgs, strOutputString);
    _vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
    //vsprintf(strBuffer, strOutputString, vlArgs);
    va_end(vlArgs);
    OutputDebugString(CA2W(strBuffer));
}

void HideWindowAndPaste(UINT uElapse) {
    if (uElapse > 0)
    {
        g_nTimerID = SetTimer(NULL, 0, 100, TimerProc); // 延迟
    }
    else
    {
        ShowWindow(hWnd, SW_HIDE);
    }
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    // 定时器到期时执行操作
    KillTimer(NULL, g_nTimerID); // 停止定时器
    ShowWindow(hWnd, SW_HIDE);

    //SetForegroundWindow(hForeWnd);
    //SetFocus(hInFocus);

    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';
    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    // 等待一会儿
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[0].ki.wVk = 'V';
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[1].ki.wVk = VK_CONTROL;
    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    //清空粘贴板
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        CloseClipboard();
    }
}
LPARAM StringToLPARAM(const std::string& str) {
    // 使用 std::stoul 将字符串转换为无符号长整型
    unsigned long ulValue = std::stoul(str);
    // 将无符号长整型转换为 LPARAM
    return static_cast<LPARAM>(ulValue);
}

// 将 std::string 转换为 TCHAR*
TCHAR* StringToTCHAR(const std::string& str) {
#ifdef UNICODE
    // 如果使用 Unicode 字符集，则将 std::string 转换为 std::wstring，然后再将其转换为 TCHAR*
    std::wstring wstr(str.begin(), str.end());
    return _wcsdup(wstr.c_str()); // 使用 _wcsdup 复制字符串
#else
    // 如果使用 ANSI 字符集，则直接将 std::string 转换为 TCHAR*
    return _strdup(str.c_str()); // 使用 _strdup 复制字符串
#endif
}

std::string TCHARToString(const TCHAR* tcharString) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, tcharString, -1, NULL, 0, NULL, NULL); // 获取转换后的字符串长度
    std::string convertedString(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, tcharString, -1, &convertedString[0], size_needed, NULL, NULL); // 进行转换
    return convertedString;
}
std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}
// 解析CSV文件并存储到结构体数组中
void ParseCsvAndStore() {
    // 获取当前工作目录
    TCHAR szPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szPath);
    // 清空结构体数组缓存
    g_data.clear();
    std::wstring filePath = szPath;
    filePath += L"\\data.csv"; // 假设要访问的文件名为 example.txt
    // 打开CSV文件
    std::ifstream file(filePath);
    if (!file.is_open()) {
        MessageBox(NULL, L"无法打开文件！", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    // 逐行读取文件内容并解析
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;

        // 用逗号分割行数据
        std::vector<std::string> tokens;
        while (std::getline(iss, token, ',')) {
            tokens.push_back(token);
        }

        // 如果CSV文件的结构和结构体不匹配，这里需要根据实际情况进行修改
        if (tokens.size() >= 2) {
            SNPI snpi;
            snpi.hLabel = tokens[0];
            snpi.hValue = tokens[1];
            g_data.push_back(snpi);
        }
    }

    // 关闭文件
    file.close();
}
int SendSelectValueToWindows()
{
    LRESULT index = SendMessage(popListBox, LB_GETCURSEL, 0, 0);
    if (index != LB_ERR)
    {
        LRESULT len = SendMessage(popListBox, LB_GETTEXTLEN, index, 0);
        TCHAR* buffer = new TCHAR[len + 1];
        SendMessage(popListBox, LB_GETTEXT, index, (LPARAM)buffer);
        std::string line = TCHARToString(buffer);
        delete[] buffer;
        std::vector<std::string> tokens = splitString(line, " -> ");
        OutputDebugPrintf(" >>> hInFocus %d <<< ", hInFocus);
        if (tokens.size() >= 2)
        {
            if (OpenClipboard(NULL)) {
                EmptyClipboard();
                HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, 10);
                if (hGlobal != NULL) {
                    char* pText = (char*)GlobalLock(hGlobal);
                    std::string s = tokens[1];
                    strcpy_s(pText, s.length() + 1, s.c_str());
                    GlobalUnlock(hGlobal);
                    SetClipboardData(CF_TEXT, hGlobal);
                }
                CloseClipboard();
            }
            HideWindowAndPaste(100); // 延迟
            return 1;
        }
    }
    return 0;

}
// 键盘钩子处理函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT* pkbhs = (KBDLLHOOKSTRUCT*)lParam;
    if (nCode >= 0)
    {
        HWND hwndFocus = GetFocus();

        if (hwndFocus == popEdit || hwndFocus == popListBox || hwndFocus == hWnd)
        {
            if (wParam == WM_KEYDOWN && (pkbhs->vkCode == VK_UP || pkbhs->vkCode == VK_DOWN))
            {
                // 获取当前选中项的索引
                LRESULT selectedIndex = SendMessage(popListBox, LB_GETCURSEL, 0, 0);

                // 根据按键更新选中项的索引
                if (pkbhs->vkCode == VK_UP && selectedIndex > 0)
                {
                    selectedIndex--;
                }
                else if (pkbhs->vkCode == VK_DOWN)
                {
                    selectedIndex++;
                }

                // 更新 ListBox 中的选中项
                SendMessage(popListBox, LB_SETCURSEL, selectedIndex, 0);
            }
            if (wParam == WM_KEYDOWN && (pkbhs->vkCode == VK_RETURN))
            {
                if(SendSelectValueToWindows())
                {
                    return CallNextHookEx(NULL, 0, 0, 0);
                }
            }
        }
        if (wParam == WM_KEYDOWN)
        {

            if ((pkbhs->vkCode == 'Q' || pkbhs->vkCode == 'q') && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
            {
                OutputDebugPrintf("Ctrl+Q ");
                POINT pt;
                GetCursorPos(&pt); // 获取鼠标当前位置
                OutputDebugPrintf("hInFocus %d", hInFocus);
                SelectStrToFocus(pt);
            }
        }
    }
    // 传递给下一个钩子
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
HWND GetForegroundWindowFocus()
{
    hForeWnd = GetForegroundWindow(); //获得当前激活的窗口句柄
    DWORD dwSelfThreadId = GetCurrentThreadId(); //获取本身的线程ID
    DWORD dwForeThreadId = GetWindowThreadProcessId(hForeWnd, nullptr); //根据窗口句柄获取线程ID
    AttachThreadInput(dwForeThreadId, dwSelfThreadId, 1); //附加线程
    HWND hwndFocus = GetFocus(); // 获取当前窗口中具有焦点的控件句柄
    AttachThreadInput(dwForeThreadId, dwSelfThreadId, 0); //取消附加的线程
    return hwndFocus;
}

HWND SelectStrToFocus(POINT pt)
{
    hForeWnd = GetForegroundWindow(); //获得当前激活的窗口句柄
    DWORD dwSelfThreadId = GetCurrentThreadId(); //获取本身的线程ID
    DWORD dwForeThreadId = GetWindowThreadProcessId(hForeWnd, nullptr); //根据窗口句柄获取线程ID
    AttachThreadInput(dwForeThreadId, dwSelfThreadId, 1); //附加线程
    hInFocus = GetFocus(); // 获取当前窗口中具有焦点的控件句柄
    if (hInFocus != NULL) {
        SetWindowText(popEdit, L"");
        SendMessage(popListBox, LB_RESETCONTENT, 0, 0);
        // 添加一些示例项
        for (const auto& item : g_data) {
            std::string str = item.hLabel + " -> " + item.hValue;
            SendMessage(popListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(StringToTCHAR(str)));
        }
        OutputDebugPrintf(">>> ShowWindow");
        ShowWindow(hWnd, SW_SHOWNORMAL);
        SetWindowPos(hWnd, HWND_TOPMOST, pt.x, pt.y, WIN_W, WIN_H, SWP_NOZORDER | SWP_SHOWWINDOW);
        SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        SetFocus(popEdit);
        SetForegroundWindow(hWnd);
    }


    AttachThreadInput(dwForeThreadId, dwSelfThreadId, 0); //取消附加的线程


    return nullptr;
}

// 注册全局快捷键
void RegisterGlobalHotkey() {
    //注册全局
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
}
// 取消全局快捷键
void UnregisterGlobalHotkey() {
    if (hHook != NULL) {
        UnhookWindowsHookEx(hHook);
        hHook = NULL;
    }
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SNIPPETS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNIPPETS));

    UpdateWindow(hWnd);
    AddTrayIcon(hWnd);
    RegisterGlobalHotkey();
    ParseCsvAndStore();

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNIPPETS));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SNIPPETS);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SNIPPETS));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_POPUP,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId = LOWORD(wParam);
    switch (message)
    {
    case WM_CREATE://窗口创建时候的消息.
        popEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            10, 10, 200, 25, hWnd, NULL, GetModuleHandle(NULL), NULL);
        popListBox = CreateWindowEx(0, L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
            10, 40, 200, 250, hWnd, NULL, GetModuleHandle(NULL), NULL);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == GetDlgCtrlID(popEdit)) {
            if (HIWORD(wParam) == EN_CHANGE) {
                // 当编辑框内容改变时，根据内容过滤列表项
                TCHAR szFilter[1024];
                SendMessage(popEdit, WM_GETTEXT, sizeof(szFilter) / sizeof(TCHAR), (LPARAM)szFilter);
                szFilter[255] = '\0'; // 添加空字符结尾
                // 清空列表框
                SendMessage(popListBox, LB_RESETCONTENT, 0, 0);

                // 根据过滤条件添加匹配的列表项
                for (const auto& item : g_data) {
                    if (_tcsstr(StringToTCHAR(item.hLabel), szFilter) != NULL) {
                        std::string str = item.hLabel + " -> " + item.hValue;
                        SendMessage(popListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(StringToTCHAR(str)));
                    }
                }
            }
        }

        if (LOWORD(wParam) == GetDlgCtrlID(popListBox) && HIWORD(wParam) == LBN_SELCHANGE) {
            SendSelectValueToWindows();
        }
        // 分析菜单选择:
        switch (wmId)
        {
        case IDM_REFRESH_SNIPPETS:
            ParseCsvAndStore();
            break;
        case IDM_TRAY:
            PopupTray(hWnd, lParam);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 在此处添加使用 hdc 的任何绘图代码...
        EndPaint(hWnd, &ps);
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

// “关于”框的消息处理程序。
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

void AddTrayIcon(HWND hwnd) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = IDM_TRAY;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    nid.uCallbackMessage = WM_COMMAND;
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SNIPPETS));
    wcscpy_s(nid.szTip, L"snippets");

    Shell_NotifyIcon(NIM_ADD, &nid);

    hMenu = CreatePopupMenu();//生成菜单
    //为托盘菜单添加两个选项
    AppendMenu(hMenu, MF_STRING, IDM_REFRESH_SNIPPETS, TEXT("刷新缓存"));
    AppendMenu(hMenu, MF_STRING, IDM_EXIT, TEXT("退出"));
}

void PopupTray(HWND hwnd, LPARAM lParam) {
    POINT pt;
    switch (lParam)
    {
    case WM_RBUTTONDOWN://右击.
        SetForegroundWindow(hWnd);
        GetCursorPos(&pt);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, NULL, hWnd, NULL);
        break;
    }
}