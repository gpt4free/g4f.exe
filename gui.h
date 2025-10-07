#ifndef GUI_H
#define GUI_H

#include <windows.h>
#include <string>
#include <functional>

class LauncherGUI {
private:
    HWND hwnd;
    HWND progressBar;
    HWND statusLabel;
    HWND detailsButton;
    HWND logWindow;
    bool showingDetails;
    HINSTANCE hInstance;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LauncherGUI* instance;

    void toggleDetails();

public:
    LauncherGUI(HINSTANCE hInst);
    ~LauncherGUI();

    bool create();
    void show();
    void setProgress(int percent);
    void setStatus(const std::string& message);
    void addLog(const std::string& message);
    void close();
    
    void runInstallation(const std::function<void(LauncherGUI*)>& installFunc);
};

#endif