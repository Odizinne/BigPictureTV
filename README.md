# BigPictureTV

[![Github All Releases](https://img.shields.io/github/downloads/odizinne/bigpicturetv/total.svg)]()
[![license](https://img.shields.io/github/license/odizinne/bigpicturetv)]()

This project automates switching between a monitor and a TV when launching Steam Big Picture on Windows.  

## Overview

![image](.assets/screenshot.png)

## Requirements

**Runtime:**
- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- [AudioDeviceCmdlets](https://github.com/frgnca/AudioDeviceCmdlets) for audio output switching.
 
## Download

### Using OdizinnePackageManager (recommended)

- Install OPM

In powershell:

```powershell
Invoke-Expression (New-Object System.Net.WebClient).DownloadString('https://raw.githubusercontent.com/Odizinne/opm/refs/heads/main/opm_install.ps1')
```

- Install BigPictureTV

In powershell:

```
opm update
opm install BigPictureTV
```

BigPictureTV will be available in your start menu.

### Manual

Head to [release](https://github.com/Odizinne/BigPictureTV/releases/latest) section and grab the latest one.

Extract it and place the directory wherever you like.  
Run `BigPictureTV.exe`.

## Usage Instructions

### Window Check Rate

- **Purpose:** Adjusts the interval for checking the presence of the Steam Big Picture window.
- **Range:** 100 ms to 1000 ms (default is 1000 ms).
- **Note:** 
  - **100 ms**: Minimal delay in mode switch but higher resource usage.
  - **1000 ms**: Default setting; balances performance and resource usage.
  - **Recommendation:** Do not change if unsure.

##

### Audio Configuration

1. **Install Audio Module**
   - Click on the "Install audio module" button.
   - Wait for the installation to complete.

2. **Configure Audio Switching**
   - Uncheck `Disable audio switching` once the module is installed.

3. **Specify Audio Outputs**
   - Use short names for audio outputs.
   - BigPictureTV will identify the correct output based on keywords.
   - Keep the names brief for better accuracy.

**Example:**

If you have a headset with the full name "CORSAIR VOID ELITE Wireless Gaming Dongle," you can simply specify "Corsair" as the audio output name. BigPictureTV will detect the correct output based on this keyword.

##

### Monitor Configuration

BigPictureTV uses Windows' built-in `displayswitch.exe` for monitor switching.

#### Available Modes

- **Internal:** Only the default monitor is enabled; all other monitors are disabled.
- **Extend:** Default monitor and selected additional monitors are enabled.
- **Clone:** All monitors are enabled and mirrored.
- **External:** Default monitor is disabled; only selected monitors are enabled.

#### Setting Up External Monitor (for more than two monitors)

1. **Set External Mode**
   - Press `Win + P` and select "External".

2. **Configure Desired Monitor**
   - Go to **Windows Settings** > **System** > **Displays**.
   - Disconnect all monitors except the one you want to use in game mode.

3. **Return to Previous Mode**
   - Press `Win + P` and switch back to "Internal" or "Extend".

**Alternative Option:**
- If you prefer not to configure monitors manually, select `Clone` in BigPictureTV settings.  
This is not optimal unless you manually set a FPS limit to your TV refresh rate in games.

### Actions

- Close discord in gamemode, start discord in desktop mode.
- Disable night light in gamemode, revert to previous state in desktop mode.
- Set performance power plan in gamemode, revert to previous state in desktop mode.
- Pause media (Spotify, Youtube...) in gamemode, restore them in desktop mode.

### Advanced

About autodetect gamemode and desktop audio:

For autodetect gamemode audio, enable it only if you're using HDMI audio from your TV, and TV is off in desktop mode.  
BigPictureTV will look for new audio device connections, and since HDMI is off when screen is not used by windows, it will find it.

Autodetect desktop audio will save your default audio output right before switching, and try to restore it when going back to desktop mode.  
If your device is disconnected when going back to desktop mode, audio switch will fail.

## Credits

- [nathanbabcock](https://github.com/nathanbabcock/nightlight-cli/tree/main) for night light toggle
