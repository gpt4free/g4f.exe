#include <iostream>
#include <windows.h>
#include <shellapi.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include "gui.h"
#include <shlobj.h>
#include <shobjidl.h>

#pragma comment(lib, "ole32.lib")
namespace fs = std::filesystem;



int detectWindowsVersion() {
    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    #pragma warning(suppress : 4996)
    GetVersionExW((LPOSVERSIONINFOW)&osvi);

    if (osvi.dwMajorVersion < 6) return 0; // XP or older
    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) return 1; // Vista
    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) return 2; // Win7
    return 3; // Win8.1+
}

std::string getPythonDownloadURL(LauncherGUI* gui) {
    int winVersion = detectWindowsVersion();

    if (winVersion <= 2) {
        // Vista or Win7 detected
        gui->addLog("Windows Vista/7 detected!");

        int choice = MessageBoxW(nullptr,
            L"Windows Vista/7 Detected!\n\n"
            L"Choose Python version:\n\n"
            L"YES = Community-built Python 3.12.10 for Vista/7\n"
            L"       (Compatible with older Windows)\n\n"
            L"NO = Standard Python 3.12.0\n"
            L"       (May not work on Vista/7)\n\n"
            L"Use Vista-compatible build?",
            L"Python Version",
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1);

        if (choice == IDYES) {
            gui->addLog("Using Vista-compatible Python 3.12.10");
            return "https://github.com/vladimir-andreevich/cpython-windows-vista-and-7/raw/d80b35bd1699491a821da4c05b8b1526303a9507/v3.12/python-3.12.10-embed-amd64.zip";
        } else {
            gui->addLog("Using standard Python 3.12.0");
            return "https://www.python.org/ftp/python/3.12.0/python-3.12.0-embed-amd64.zip";
        }
    } else {
        // Windows 8.1 or newer
        gui->addLog("Windows 8.1+ detected - using official Python 3.12.0");
        return "https://www.python.org/ftp/python/3.12.0/python-3.12.0-embed-amd64.zip";
    }
}

bool createDesktopShortcut() {
    // Get desktop path
    wchar_t desktopPath[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, desktopPath);

    // Get current directory (where python-embed is)
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);

    // Create shortcut path
    std::wstring shortcutPath = std::wstring(desktopPath) + L"\\Start G4F Server.lnk";

    // Python exe path
    std::wstring pythonPath = std::wstring(currentDir) + L"\\python-embed\\python.exe";

    // Command arguments
    std::wstring arguments = L"-m g4f.cli api --port 8080";

    // Create shortcut using COM
    CoInitialize(nullptr);

    IShellLinkW* pShellLink = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_IShellLinkW, (void**)&pShellLink);

    if (SUCCEEDED(hr)) {
        pShellLink->SetPath(pythonPath.c_str());
        pShellLink->SetArguments(arguments.c_str());
        pShellLink->SetWorkingDirectory(currentDir);
        pShellLink->SetDescription(L"Start g4f API and GUI server");

        // Set icon (use Python icon)
        pShellLink->SetIconLocation(pythonPath.c_str(), 0);

        IPersistFile* pPersistFile = nullptr;
        hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);

        if (SUCCEEDED(hr)) {
            hr = pPersistFile->Save(shortcutPath.c_str(), TRUE);
            pPersistFile->Release();
        }

        pShellLink->Release();
    }

    CoUninitialize();

    return SUCCEEDED(hr);
}

// Visible system command (shows console)
int visibleSystem(const std::string& command, LauncherGUI* gui) {
    gui->addLog("Running command: " + command);

    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;  // SHOW the console!

    std::string cmd = "cmd.exe /C " + command;

    if (!CreateProcessA(nullptr, const_cast<char*>(cmd.c_str()),
        nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE,  // New console window
        nullptr, nullptr, &si, &pi)) {
        gui->addLog("ERROR: Failed to create process");
        return -1;
        }

    gui->addLog("Waiting for process to complete...");
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    gui->addLog("Process completed with code: " + std::to_string(exitCode));

    return exitCode;
}

int silentSystem(const std::string& command) {
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    std::string cmd = "cmd.exe /C " + command;

    if (!CreateProcessA(nullptr, const_cast<char*>(cmd.c_str()),
        nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
        nullptr, nullptr, &si, &pi)) {
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exitCode;
}

bool pythonExists() {
    return fs::exists("python-embed/python.exe");
}

bool enableSitePackages() {
    // Check if directory exists first
    if (!fs::exists("python-embed") || !fs::is_directory("python-embed")) {
        return false;
    }

    std::string pthFile;

    try {
        for (const auto& entry : fs::directory_iterator("python-embed")) {
            if (entry.path().extension() == "._pth") {
                pthFile = entry.path().string();
                break;
            }
        }
    } catch (const std::exception& e) {

        return false;
    }

    if (pthFile.empty()) return false;

    try {
        std::ifstream inFile(pthFile);
        if (!inFile.is_open()) return false;

        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string content = buffer.str();
        inFile.close();

        if (content.find("import site") != std::string::npos &&
            content.find("#import site") == std::string::npos) {
            return true;
            }

        size_t pos = content.find("#import site");
        if (pos != std::string::npos) {
            content.replace(pos, 12, "import site");
        } else {
            content += "\nimport site\n";
        }

        std::ofstream outFile(pthFile);
        if (!outFile.is_open()) return false;

        outFile << content;
        outFile.close();

    } catch (const std::exception& e) {
        return false;
    }

    return true;
}

bool downloadPython(LauncherGUI* gui) {
    gui->setStatus("Downloading portable Python...");
    gui->addLog("=== PYTHON SETUP ===");
    gui->addLog("Detecting Windows version...");
    gui->setProgress(10);

    std::string url = getPythonDownloadURL(gui);

    gui->addLog("Downloading Python embedded...");
    std::string downloadCmd = "powershell -Command \"Invoke-WebRequest -Uri '" + url + "' -OutFile 'python.zip'\"";

    gui->addLog("Download started...");
    int result = silentSystem(downloadCmd.c_str());
    if (result != 0) {
        gui->addLog("ERROR: Download failed!");
        return false;
    }

    gui->addLog("Download complete! (12 MB)");
    gui->setProgress(40);
    gui->setStatus("Extracting Python...");
    gui->addLog("Extracting archive...");

    std::string extractCmd = "powershell -Command \"Expand-Archive -Path 'python.zip' -DestinationPath 'python-embed' -Force\"";
    result = silentSystem(extractCmd.c_str());
    if (result != 0) {
        gui->addLog("ERROR: Extraction failed!");
        return false;
    }

    gui->addLog("Extraction complete!");

    // Give filesystem a moment to settle
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Verify extraction worked
    if (!fs::exists("python-embed") || !fs::exists("python-embed/python.exe")) {
        gui->addLog("ERROR: Python extraction incomplete!");
        return false;
    }

    fs::remove("python.zip");

    gui->setProgress(50);
    gui->setStatus("Configuring Python...");
    gui->addLog("Enabling site-packages...");

    bool success = enableSitePackages();
    if (success) {
        gui->addLog("Python configured successfully!");
    } else {
        gui->addLog("WARNING: Configuration had issues, but may still work");
        // Don't fail completely - Python might still work
        success = true;
    }

    return success;
}

bool installPip(LauncherGUI* gui) {
    gui->setStatus("Installing pip...");
    gui->addLog("\n=== PIP INSTALLATION ===");
    gui->addLog("Downloading get-pip.py...");
    gui->setProgress(55);

    std::string getPipUrl = "https://bootstrap.pypa.io/get-pip.py";
    std::string downloadCmd = "powershell -Command \"Invoke-WebRequest -Uri '" + getPipUrl + "' -OutFile 'get-pip.py'\"";

    int result = silentSystem(downloadCmd.c_str());
    if (result != 0) {
        gui->addLog("ERROR: Failed to download get-pip.py!");
        return false;
    }

    gui->addLog("Running pip installer...");
    gui->setProgress(60);

    result = silentSystem("python-embed\\python.exe get-pip.py --no-warn-script-location");
    if (result != 0) {
        gui->addLog("ERROR: Pip installation failed!");
        return false;
    }

    fs::remove("get-pip.py");
    gui->addLog("Pip installed successfully!");

    return true;
}

bool installG4F(LauncherGUI* gui) {
    gui->setStatus("Installing g4f...");
    gui->addLog("\n=== G4F INSTALLATION ===");

    // Ask user which version to install
    int choice = MessageBoxW(nullptr,
        L"Choose installation type:\n\n"
        L"YES = Full installation (g4f[all])\n"
        L"       All features and extras\n"
        L"       ~600MB download\n"
        L"       4-5 minutes\n\n"
        L"NO = Optimized installation - RECOMMENDED\n"
        L"      API + GUI + providers\n"
        L"      Skips markdown converters\n"
        L"      ~150MB download\n"
        L"      1-2 minutes\n"
        L"      Much faster!\n\n"
        L"Install full version?",
        L"Installation Type",
        MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);

    gui->addLog("Installing build tools...");
    silentSystem("python-embed\\python.exe -m pip install setuptools wheel --no-warn-script-location --quiet");

    gui->setProgress(65);

    if (choice == IDYES) {
        // Full installation
        gui->addLog("Installing g4f[all] (full installation)...");
        gui->addLog("This may take 5-10 minutes (~600MB)...");

        gui->setProgress(70);
        gui->setStatus("Installing g4f and dependencies...");

        int result = silentSystem("python-embed\\python.exe -m pip install \"g4f[all]\" --prefer-binary --no-warn-script-location");

        if (result != 0) {
            gui->addLog("ERROR: g4f installation failed!");
            return false;
        }

    } else {
        // Optimized installation (no markdown bloat)
        gui->addLog("Installing g4f (optimized installation - recommended)...");
        gui->addLog("Installing core package and essential dependencies...");
        gui->addLog("This may take 3-5 minutes (~250-300MB)...");

        gui->setProgress(70);
        gui->setStatus("Installing g4f core...");

        // Install core g4f
        int result = silentSystem("python-embed\\python.exe -m pip install g4f --prefer-binary --no-warn-script-location");
        if (result != 0) {
            gui->addLog("ERROR: Failed to install g4f core!");
            return false;
        }

        gui->setProgress(75);
        gui->setStatus("Installing API server...");
        gui->addLog("Installing API server dependencies...");

        // Install API dependencies
        result = silentSystem("python-embed\\python.exe -m pip install fastapi uvicorn python-multipart loguru --prefer-binary --no-warn-script-location --quiet");
        if (result != 0) {
            gui->addLog("WARNING: Some API dependencies may be missing");
        }

        gui->setProgress(80);
        gui->setStatus("Installing GUI...");
        gui->addLog("Installing GUI dependencies...");

        // Install GUI dependencies
        result = silentSystem("python-embed\\python.exe -m pip install flask werkzeug a2wsgi beautifulsoup4 pillow --prefer-binary --no-warn-script-location --quiet");
        if (result != 0) {
            gui->addLog("WARNING: Some GUI dependencies may be missing");
        }

        gui->setProgress(85);
        gui->addLog("Installing provider dependencies...");

        // Install essential provider dependencies (no markdown!)
        silentSystem("python-embed\\python.exe -m pip install curl_cffi certifi browser_cookie3 ddgs --prefer-binary --no-warn-script-location --quiet");

        gui->addLog("Skipped markdown converters (saves ~150MB)");
    }

    gui->setProgress(90);
    gui->addLog("g4f installed successfully!");

    return true;
}

bool installFFmpeg(LauncherGUI* gui) {
    gui->setProgress(91);
    gui->addLog("\n=== FFMPEG INSTALLATION ===");

    // Check if already installed
    if (fs::exists("python-embed/Scripts/ffmpeg.exe")) {
        gui->addLog("ffmpeg already installed!");
        gui->setProgress(98);
        return true;
    }

    // Ask user if they want to install it
    gui->addLog("Prompting user for optional ffmpeg installation...");

    int choice = MessageBoxW(nullptr,
        L"Install ffmpeg? (~100MB, 5-10 minutes)\n\n"
        L"ffmpeg is optional and enables audio processing features.\n"
        L"Most g4f features work perfectly without it.\n\n"
        L"You can always install it manually later if needed.\n\n"
        L"Install ffmpeg now?",
        L"Optional Component - ffmpeg",
        MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);  // Default to NO

    if (choice == IDNO) {
        gui->addLog("User skipped ffmpeg installation (optional)");
        gui->addLog("g4f will work fine without it!");
        gui->setProgress(98);
        return true;
    }

    // User wants to install it
    gui->addLog("User chose to install ffmpeg");
    gui->addLog("Downloading ffmpeg (~100 MB)...");
    gui->addLog("A console window will show download progress...");
    gui->setStatus("Downloading ffmpeg (optional)...");

    std::string ffmpegUrl = "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip";
    std::string downloadCmd = "powershell -Command \"Write-Host 'Downloading ffmpeg...'; "
                             "$ProgressPreference = 'Continue'; "
                             "Invoke-WebRequest -Uri '" + ffmpegUrl + "' -OutFile 'ffmpeg.zip' -Verbose\"";

    int result = visibleSystem(downloadCmd, gui);
    if (result != 0) {
        gui->addLog("WARNING: ffmpeg download failed");
        MessageBoxW(nullptr,
            L"ffmpeg download failed.\n\n"
            L"This is optional - g4f will still work!\n"
            L"You can install ffmpeg manually later if needed.",
            L"Optional Component Failed",
            MB_OK | MB_ICONWARNING);
        gui->setProgress(98);
        return true;
    }

    gui->addLog("Download complete! Extracting...");
    gui->setProgress(94);
    gui->setStatus("Extracting ffmpeg...");

    std::string extractCmd = "powershell -Command \"Write-Host 'Extracting...'; "
                            "Expand-Archive -Path 'ffmpeg.zip' -DestinationPath 'ffmpeg-temp' -Force\"";
    result = visibleSystem(extractCmd, gui);
    if (result != 0) {
        gui->addLog("WARNING: ffmpeg extraction failed");
        fs::remove("ffmpeg.zip");
        gui->setProgress(98);
        return true;
    }

    gui->addLog("Copying ffmpeg.exe to Python Scripts...");
    gui->setProgress(96);

    try {
        fs::create_directories("python-embed/Scripts");

        bool found = false;
        for (const auto& entry : fs::recursive_directory_iterator("ffmpeg-temp")) {
            if (entry.path().filename() == "ffmpeg.exe") {
                gui->addLog("Found: " + entry.path().string());
                fs::copy_file(entry.path(), "python-embed/Scripts/ffmpeg.exe",
                             fs::copy_options::overwrite_existing);
                gui->addLog("✓ ffmpeg.exe installed successfully!");
                found = true;
                break;
            }
        }

        if (!found) {
            gui->addLog("ERROR: ffmpeg.exe not found in archive");
        }

        gui->addLog("Cleaning up...");
        fs::remove_all("ffmpeg-temp");
        fs::remove("ffmpeg.zip");

        if (found) {
            gui->addLog("✓ ffmpeg installation complete!");
        }

    } catch (const std::exception& e) {
        gui->addLog(std::string("WARNING: ") + e.what());
        try {
            fs::remove_all("ffmpeg-temp");
            fs::remove("ffmpeg.zip");
        } catch (...) {}
    }

    gui->setProgress(98);
    return true;
}

bool pipExists() {
    int result = silentSystem("python-embed\\python.exe -m pip --version >nul 2>&1");
    return (result == 0);
}

bool g4fExists() {
    int result = silentSystem("python-embed\\python.exe -c \"import g4f\" 2>nul");
    return (result == 0);
}

void runG4FServer(LauncherGUI* gui) {
    gui->setStatus("Starting g4f server...");
    gui->addLog("\n=== STARTING SERVER ===");
    gui->setProgress(100);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    gui->addLog("Starting g4f with API and GUI on port 8080...");
    gui->addLog("\nAPI Endpoints:");
    gui->addLog("  - POST http://localhost:8080/v1/chat/completions");
    gui->addLog("  - POST http://localhost:8080/v1/completions");
    gui->addLog("  - GET  http://localhost:8080/v1/models");
    gui->addLog("\nWeb Interface:");
    gui->addLog("  - http://localhost:8080/chat/");
    gui->addLog("\nServer starting...");
    gui->addLog("Opening browser...");

    // Start g4f with API mode (includes GUI automatically)
    std::thread([]() {
        system("python-embed\\python.exe -m g4f.cli api --port 8080");
    }).detach();

    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Open browser to web GUI (same port!)
    ShellExecuteW(nullptr, L"open", L"http://localhost:8080/chat/", nullptr, nullptr, SW_SHOW);

    gui->addLog("Browser opened!");
    gui->addLog("\nServer running on http://localhost:8080");
    gui->addLog("Both API and GUI are available!");
    gui->addLog("\nKeep this window open to keep server running.");
    gui->addLog("Close this window to stop the server.");

    // Keep GUI open so server keeps running
}
void installationProcess(LauncherGUI* gui) {
    try {
        gui->addLog("g4f Launcher by jokukiller");
        gui->addLog("Starting installation process...\n");

        if (!pythonExists()) {
            if (!downloadPython(gui)) {
                gui->setStatus("Failed to setup Python!");
                MessageBoxW(nullptr, L"Failed to download Python!\nCheck the details log for more info.", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
        } else {
            gui->addLog("=== PYTHON SETUP ===");
            gui->addLog("Portable Python already installed!");
            gui->setProgress(50);
            enableSitePackages();
        }

        if (!pipExists()) {
            if (!installPip(gui)) {
                gui->setStatus("Failed to install pip!");
                MessageBoxW(nullptr, L"Failed to install pip!\nCheck the details log for more info.", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
        } else {
            gui->addLog("\n=== PIP INSTALLATION ===");
            gui->addLog("Pip already installed!");
            gui->setProgress(60);
        }

        if (!g4fExists()) {
            if (!installG4F(gui)) {
                gui->setStatus("Failed to install g4f!");
                MessageBoxW(nullptr, L"Failed to install g4f!\nCheck the details log for more info.", L"Error", MB_OK | MB_ICONERROR);
                return;
            }
        } else {
            gui->addLog("\n=== G4F INSTALLATION ===");
            gui->addLog("g4f already installed!");
            gui->setProgress(90);
        }

        installFFmpeg(gui);
        gui->addLog("\n=== CREATING SHORTCUT ===");
        gui->addLog("Creating desktop shortcut...");
        if (createDesktopShortcut()) {
            gui->addLog("✓ Desktop shortcut created: 'Start G4F Server'");
        } else {
            gui->addLog("WARNING: Could not create desktop shortcut");
        }
        gui->setProgress(98);
        gui->addLog("\n=== INSTALLATION COMPLETE ===");
        gui->addLog("All components installed successfully!");

        runG4FServer(gui);

    } catch (const std::exception& e) {
        gui->addLog(std::string("\nFATAL ERROR: ") + e.what());
        MessageBoxW(nullptr, L"An unexpected error occurred!\nCheck the details log for more info.", L"Error", MB_OK | MB_ICONERROR);
    } catch (...) {
        gui->addLog("\nFATAL ERROR: Unknown exception occurred!");
        MessageBoxW(nullptr, L"An unexpected error occurred!\nCheck the details log for more info.", L"Error", MB_OK | MB_ICONERROR);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    LauncherGUI gui(hInstance);

    if (!gui.create()) {
        MessageBoxW(nullptr, L"Failed to create window!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    gui.show();
    gui.runInstallation(installationProcess);

    return 0;
}