# g4f.exe - Official Windows Launcher

**Official** one-click installer for [g4f (gpt4free)](https://github.com/xtekky/gpt4free)!

![Version](https://img.shields.io/badge/version-1.0-blue)
![Size](https://img.shields.io/badge/install%20size-149MB-green)
![Platform](https://img.shields.io/badge/platform-Windows-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## ✨ Features

- 🚀 **One-click installation** - No technical knowledge needed
- 📦 **Portable** - No system changes, everything self-contained
- 💾 **Optimized** - Only 149 MB installed (75% smaller than full install)
- 🌐 **API + GUI** - Both web interface and REST API included
- 🖥️ **Wide compatibility** - Windows Vista/7/8.1/10/11 support
- 🔗 **Desktop shortcut** - Easy server startup
- ⚡ **Fast** - 2-5 minute installation

## 📥 Download

**[Download Latest Release](https://github.com/gpt4free/g4f.exe/releases/latest)**
## 📥 Download Options

### Option 1: Installer (Recommended)
**[Download g4f_launcher.exe](https://github.com/gpt4free/g4f.exe/releases/download/ver1/g4f_launcher.exe)** (~100 KB)

- Automatically downloads and installs Python + g4f
- Choose Optimized (149 MB) or Full (600 MB)

### Option 2: Portable (Pre-installed)
**[Download gpt4free_portable_preinstalled.zip](https://github.com/gpt4free/g4f.exe/releases/download/ver1/gpt4free_portable_preinstalled.zip)** (~600 MB)

- Python and g4f already installed
- Just extract and run
- No installation needed
- This is for when youre having trouble with option 1






Or build from source (see below)

## 🎯 System Requirements

### Recommended
- **Windows 10** or **Windows 11** (64-bit)
- 4 GB RAM
- 500 MB free disk space

### Minimum
- **Windows 8.1** (64-bit)
- 2 GB RAM
- 200 MB free disk space

### ⚠️ Windows Vista/7 (Experimental Support)

Windows Vista and 7 have **limited support** due to outdated TLS/SSL:

**What Works:**
- ✅ Launcher detects Vista/7 automatically
- ✅ Offers community-built Python 3.12.10 (Vista/7 compatible)
- ✅ All g4f features work once installed

**Known Limitations:**
- ❌ **Automatic downloads may fail** due to TLS 1.0/1.1 limitations
- ❌ Vista/7 cannot connect to modern HTTPS sites (GitHub, python.org require TLS 1.2+)
- ❌ Microsoft ended support (Vista: 2017, Win7: 2020)

**Manual Installation Workaround:**

If automatic downloads fail on Vista/7:

1. **On a modern computer**, download:
   - Python: https://github.com/vladimir-andreevich/cpython-windows-vista-and-7/raw/main/v3.12/python-3.12.10-embed-amd64.zip

2. **Transfer to Vista/7** (USB, network, etc.)

3. **Extract to `python-embed` folder** next to `g4f_launcher.exe`

4. **Run launcher again** - continues installation automatically

See [VISTA-SUPPORT.md](VISTA-SUPPORT.md) for detailed instructions.

**Recommendation:** Upgrade to Windows 10/11 for better security and compatibility.

## 🚀 Quick Start

1. **Download** `g4f_launcher.exe` from [releases](https://github.com/gpt4free/g4f.exe/releases/latest)
2. **Run** the executable
3. **Choose** installation type:
   - **Optimized** (149 MB) ⭐ Recommended - API + GUI + all providers
   - **Full** (600 MB) - Everything + extras (markdown converters, etc.)
4. **Wait** for installation (~1-2 minutes for optimized)
5. **Browser opens** automatically to http://localhost:8080/chat/
6. **Start chatting!** 🎉

## 📊 Installation Types

| Type | Size | Install Time | What's Included |
|------|------|--------------|-----------------|
| **Optimized** ⭐ | 149 MB | 1-2 min | API, GUI, all providers, essential dependencies |
| Full | 600 MB | 4-5 min | Everything + markdown converters & extras |

### What's Skipped in Optimized?
- Markdown file converters (Word/Excel/PDF → markdown)
- Browser automation (nodriver)
- Desktop webview components
- Other rarely-used extras

**All chat, API, and provider features work perfectly in Optimized mode!**

## 🛠️ What Gets Installed

- ✅ Portable Python 3.12 (3.12.10 for Vista/7)
- ✅ g4f (gpt4free) package
- ✅ FastAPI + Uvicorn (API server)
- ✅ Flask + Werkzeug (Web GUI)
- ✅ All provider dependencies
- ✅ Desktop shortcut: "Start G4F Server"

**Everything is portable** - no system changes, no registry entries!

## 🎮 Usage

### Starting the Server

**Method 1:** Double-click **"Start G4F Server"** desktop shortcut

**Method 2:** Run manually:
```cmd
cd path\to\g4f
python-embed\python.exe -m g4f.cli api --port 8080
```

### Accessing g4f

- **Web GUI:** http://localhost:8080/chat/
- **API Endpoint:** http://localhost:8080/v1/chat/completions
- **OpenAI-compatible API:** http://localhost:8080/v1/
- **Models List:** http://localhost:8080/v1/models

### API Example

```bash
curl http://localhost:8080/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "gpt-4",
    "messages": [{"role": "user", "content": "Hello!"}]
  }'
```

### Stopping the Server

Close the server console window or press `Ctrl+C`

## 🔧 Troubleshooting

### Server won't start
- Check if port 8080 is in use: `netstat -ano | findstr :8080`
- Try different port: `python-embed\python.exe -m g4f.cli api --port 8081`
- Check Windows Firewall settings

### "Module not found" errors
Reinstall dependencies:
```cmd
python-embed\python.exe -m pip install g4f
python-embed\python.exe -m pip install fastapi uvicorn flask werkzeug a2wsgi
```

### Browser doesn't open automatically
Manually navigate to: http://localhost:8080/chat/

### Vista/7 download failures
See [VISTA-SUPPORT.md](VISTA-SUPPORT.md) for manual installation steps.

## 🏗️ Building from Source

### Prerequisites
- Windows 10/11
- Visual Studio 2019 or newer (with C++ tools)
- CMake 3.10+
- Git

### Build Steps

```bash
# Clone repository
git clone https://github.com/gpt4free/g4f.exe.git
cd g4f.exe

# Build with CMake
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Output: g4f.exe in build/Release/
```

### Using CLion (Recommended)
1. Open project in CLion
2. Select **Release** configuration
3. Build → Build Project
4. Output in `cmake-build-release/`

## 📁 Project Structure

```
g4f.exe/
├── main.cpp           # Main launcher logic
├── gui.cpp            # GUI implementation
├── gui.h              # GUI header
├── version.rc         # Version info & resources
├── CMakeLists.txt     # Build configuration
├── README.md          # This file
└── VISTA-SUPPORT.md   # Vista/7 detailed guide
```

## 🤝 Contributing

Contributions are welcome!

1. Fork the repository
2. Create feature branch: `git checkout -b feature-name`
3. Commit changes: `git commit -am 'Add feature'`
4. Push to branch: `git push origin feature-name`
5. Submit Pull Request

## 🙏 Credits

- **g4f (gpt4free)** - [H Lohaus (xtekky)](https://github.com/xtekky/gpt4free)
- **Launcher developer** - [jokukiller](https://github.com/jokukiller)
- **Vista Python** - [vladimir-andreevich](https://github.com/vladimir-andreevich/cpython-windows-vista-and-7)

## 📝 License

MIT License - See [LICENSE](LICENSE) file for details

## ⚠️ Disclaimer

This is the **official Windows launcher** for g4f.

For the main g4f project, visit: https://github.com/xtekky/gpt4free

## 🐛 Issues & Support

- **Bug reports:** [Open an issue](https://github.com/gpt4free/g4f.exe/issues)
- **g4f Discord:** [Join server](https://discord.gg/uZQBESpFrp)
- **Discussions:** [GitHub Discussions](https://github.com/xtekky/gpt4free/discussions)

---

**Part of the [gpt4free](https://github.com/xtekky/gpt4free) project** 💜

**Star ⭐ the repo if you find it useful!**
