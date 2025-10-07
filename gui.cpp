#include "gui.h"
#include <commctrl.h>
#include <thread>

#pragma comment(lib, "comctl32.lib")

LauncherGUI* LauncherGUI::instance = nullptr;

LauncherGUI::LauncherGUI(HINSTANCE hInst)
    : hInstance(hInst), hwnd(nullptr), progressBar(nullptr),
      statusLabel(nullptr), detailsButton(nullptr), logWindow(nullptr),
      showingDetails(false) {

    instance = this;

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);
}

LauncherGUI::~LauncherGUI() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
    instance = nullptr;
}

void LauncherGUI::toggleDetails() {
    showingDetails = !showingDetails;

    if (showingDetails) {
        SetWindowPos(hwnd, nullptr, 0, 0, 550, 450, SWP_NOMOVE | SWP_NOZORDER);
        ShowWindow(logWindow, SW_SHOW);
        SetWindowTextW(detailsButton, L"Hide Details ");
    } else {
        SetWindowPos(hwnd, nullptr, 0, 0, 550, 220, SWP_NOMOVE | SWP_NOZORDER);
        ShowWindow(logWindow, SW_HIDE);
        SetWindowTextW(detailsButton, L"Show Details ");
    }
}

LRESULT CALLBACK LauncherGUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (instance) {
                    instance->toggleDetails();
                }
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool LauncherGUI::create() {
    const wchar_t CLASS_NAME[] = L"G4FLauncherClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 250));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));

    RegisterClassW(&wc);

    hwnd = CreateWindowExW(
        0, CLASS_NAME, L"gpt4free",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 550, 220,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) return false;

    HFONT hTitleFont = CreateFontW(
        28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    HFONT hFont = CreateFontW(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    HFONT hMonoFont = CreateFontW(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas"
    );

    HWND titleLabel = CreateWindowW(
        L"STATIC", L"g4f Launcher",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        75, 20, 400, 40,
        hwnd, nullptr, hInstance, nullptr
    );
    SendMessage(titleLabel, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

    statusLabel = CreateWindowW(
        L"STATIC", L"Initializing...",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        75, 70, 400, 25,
        hwnd, nullptr, hInstance, nullptr
    );
    SendMessage(statusLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

    progressBar = CreateWindowExW(
        0, PROGRESS_CLASSW, nullptr,
        WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
        75, 105, 400, 30,
        hwnd, nullptr, hInstance, nullptr
    );
    SendMessage(progressBar, PBM_SETBARCOLOR, 0, RGB(138, 99, 210));
    SendMessage(progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    detailsButton = CreateWindowW(
        L"BUTTON", L"Show Details ",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 145, 150, 30,
        hwnd, (HMENU)1, hInstance, nullptr
    );
    SendMessage(detailsButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    HWND footerLabel = CreateWindowW(
        L"STATIC", L"by jokukiller",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        75, 180, 400, 25,
        hwnd, nullptr, hInstance, nullptr
    );
    SendMessage(footerLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

    logWindow = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        20, 220, 510, 200,
        hwnd, nullptr, hInstance, nullptr
    );
    SendMessage(logWindow, WM_SETFONT, (WPARAM)hMonoFont, TRUE);

    return true;
}

void LauncherGUI::show() {
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

void LauncherGUI::setProgress(int percent) {
    if (progressBar) {
        SendMessage(progressBar, PBM_SETPOS, percent, 0);
    }
}

void LauncherGUI::setStatus(const std::string& message) {
    if (statusLabel) {
        std::wstring wideMsg(message.begin(), message.end());
        SetWindowTextW(statusLabel, wideMsg.c_str());
    }
}

void LauncherGUI::addLog(const std::string& message) {
    if (logWindow) {
        int len = GetWindowTextLengthW(logWindow);
        SendMessage(logWindow, EM_SETSEL, len, len);

        std::wstring wideMsg(message.begin(), message.end());
        wideMsg += L"\r\n";
        SendMessageW(logWindow, EM_REPLACESEL, FALSE, (LPARAM)wideMsg.c_str());
    }
}

void LauncherGUI::close() {
    if (hwnd) {
        PostQuitMessage(0);
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void LauncherGUI::runInstallation(const std::function<void(LauncherGUI*)>& installFunc) {
    std::thread([this, installFunc]() {
        installFunc(this);
    }).detach();

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}