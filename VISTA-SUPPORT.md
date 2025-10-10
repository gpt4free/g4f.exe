# Windows Vista/7 Support Guide

## Overview

Windows Vista and 7 are **experimentally supported** but have significant limitations due to their age and outdated security protocols.

### Key Facts
- 🕰️ **Vista released:** January 2007 (18 years ago)
- 🕰️ **Vista support ended:** April 11, 2017 (8 years ago)
- 🕰️ **Windows 7 support ended:** January 14, 2020 (5 years ago)
- ⚠️ **Security risk:** No more security updates from Microsoft

### Important Note
**g4f requires an internet connection** to access AI providers. This is an online-only service.

## Why Automatic Installation May Fail

### TLS/SSL Version Problem

Modern websites (including GitHub, python.org, and PyPI) require **TLS 1.2 or higher** for secure connections:

| Windows Version | Default TLS Support | Can Download from Modern Sites? |
|----------------|---------------------|--------------------------------|
| Vista RTM | TLS 1.0 only | ❌ No |
| Vista SP2 | TLS 1.0, 1.1 (with updates) | ❌ No |
| Windows 7 | TLS 1.0, 1.1 | ❌ No |
| Windows 8.1+ | TLS 1.2, 1.3 | ✅ Yes |

**Result:** Vista and Windows 7 cannot connect to modern HTTPS websites that enforce TLS 1.2+

### What This Means

When the launcher tries to download Python or g4f packages:

```
Vista/7 → "Hello, I speak TLS 1.0 and 1.1"
GitHub/PyPI → "Sorry, minimum requirement is TLS 1.2"
Vista/7 → "I don't understand TLS 1.2..."
Server → *Connection refused* ❌
```

## Manual Installation Guide

If automatic installation fails due to TLS limitations: 
First try the zip [https://github.com/gpt4free/g4f.exe/releases/download/v1.0.1-2/vista_compatibl_g4f.zip] 
if the zip also does not work. 
then try manual install: 

### Step 1: Download Files on Modern Computer

On a Windows 10/11 computer:

#### Download Python for Vista/7:
- **URL:** https://github.com/vladimir-andreevich/cpython-windows-vista-and-7/raw/main/v3.12/python-3.12.10-embed-amd64.zip
- **Size:** ~11 MB
- **Note:** Community-built Python 3.12.10 with Vista/7 compatibility

#### Download g4f packages:

```bash
# On modern computer with Python installed:
mkdir g4f-packages
pip download g4f -d g4f-packages
pip download fastapi uvicorn flask werkzeug a2wsgi beautifulsoup4 pillow -d g4f-packages
pip download curl_cffi certifi browser_cookie3 ddgs loguru python-multipart -d g4f-packages
```

This downloads all required `.whl` files.

### Step 2: Transfer Files to Vista/7

Use USB drive or network share to transfer:
- `python-3.12.10-embed-amd64.zip`
- `g4f-packages` folder
- `g4f.exe` launcher

### Step 3: Install on Vista/7

1. **Extract Python:**
   - Unzip `python-3.12.10-embed-amd64.zip`
   - Rename the folder to `python-embed`
   - Place it in the same folder as `g4f.exe`

2. **Open Command Prompt:**
   - Press `Win+R`
   - Type: `cmd`
   - Press Enter

3. **Navigate to launcher folder:**
   ```cmd
   cd C:\path\to\g4f.exe\folder
   ```

4. **Install pip packages:**
   ```cmd
   python-embed\python.exe -m pip install --no-index --find-links=g4f-packages g4f
   python-embed\python.exe -m pip install --no-index --find-links=g4f-packages fastapi uvicorn flask werkzeug a2wsgi beautifulsoup4 pillow curl_cffi certifi browser_cookie3 ddgs loguru python-multipart
   ```

5. **Verify installation:**
   ```cmd
   python-embed\python.exe -c "import g4f; print(g4f.__version__)"
   ```

   Should print the g4f version number.

### Step 4: Start Server

```cmd
python-embed\python.exe -m g4f.cli api --port 8080
```

Or double-click the **"Start G4F Server"** shortcut if it was created.

Open your browser to: **http://localhost:8080/chat/**

**Note:** You need an internet connection on the Vista/7 machine to actually use g4f (it connects to AI providers online).

## Troubleshooting

### Error: "No module named 'g4f'"

**Solution:** g4f isn't installed. Follow Step 3 above to install from downloaded packages.

### Error: "SSL: CERTIFICATE_VERIFY_FAILED"

**Cause:** Vista/7's outdated certificate store.

**Solution:** Some providers may not work. Try different providers that accept older TLS versions.

### Port 8080 Already in Use

**Solution:** Use a different port:
```cmd
python-embed\python.exe -m g4f.cli api --port 8081
```

Then access: http://localhost:8081/chat/

### "Python312.dll not found"

**Solution:** You need the Vista-compatible Python build, not the standard one.

Download from: https://github.com/vladimir-andreevich/cpython-windows-vista-and-7/

### Providers Not Working

**Issue:** Some AI providers enforce TLS 1.2+ and won't work on Vista/7.

**Solution:** 
- Try different providers in the GUI
- Some providers may have legacy endpoints that still support TLS 1.0/1.1
- Consider upgrading your OS for full compatibility

## Performance Notes

### Expected Performance on Vista/7:

- **Server startup:** ~10-15 seconds
- **Response time:** Depends on internet connection and selected provider
- **RAM usage:** ~200-300 MB
- **Some providers may not work** due to TLS restrictions

### Tips for Better Performance:

- ✅ Close unnecessary programs
- ✅ Use lightweight providers
- ✅ Disable Windows visual effects
- ✅ Ensure at least 2 GB free RAM

## Community Resources

- **g4f Discord:** https://discord.gg/uZQBESpFrp
- **Report Issues:** https://github.com/gpt4free/g4f.exe/issues
- **Vista Python:** https://github.com/vladimir-andreevich/cpython-windows-vista-and-7
- **Main g4f Repository:** https://github.com/xtekky/gpt4free

## FAQ

**Q: Can I use g4f offline?**  
A: No. g4f connects to online AI providers. You need an active internet connection.

**Q: Will Vista support be removed in the future?**  
A: Possibly. As Vista/7 usage drops, maintaining compatibility becomes difficult.

**Q: Can I use g4f.exe on Windows XP?**  
A: No. Python 3.12 requires Windows Vista or newer.

**Q: Why does download fail with "SSL error"?**  
A: Vista/7 use outdated TLS 1.0/1.1. Modern sites require TLS 1.2+. Follow the manual installation guide.

**Q: Is it safe to use g4f on Vista?**  
A: g4f itself is safe, but running **any online software** on Vista/7 is risky due to OS vulnerabilities and no security updates.

**Q: Which providers work on Vista/7?**  
A: It depends. Providers that enforce TLS 1.2+ won't work. Try different providers in the GUI to find compatible ones.

---

**Bottom line:** Vista/7 support exists for installation, but many providers may not work due to TLS limitations. Upgrading your OS is strongly recommended.

**Made with 💜 for the g4f community**
