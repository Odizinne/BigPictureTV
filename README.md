# BigPictureTV

This Python project automates switching between a monitor and a TV when launching Steam Big Picture on Windows.

![image](https://github.com/Odizinne/BigPictureTV/assets/102679854/c9713a79-b9c9-41f3-8c2a-87fa584dcf41) ![image](https://github.com/Odizinne/BigPictureTV/assets/102679854/ac5ad5fb-3113-4121-bc6e-8cf3862b63be)



/!\ Exe built with pyinstaller. Windows defender may flag it. Either add an exception or do not use it. /!\

## Download

Head to [release](https://github.com/Odizinne/BigPictureTV/releases) section and grab the latest one.

## Requirements

**Runtime:**
- [AudioDeviceCmdlets](https://github.com/frgnca/AudioDeviceCmdlets) for audio output switching.
 
**Build dependencies:**
- Python
- PyQt5 (Ui)
- winshell (Startup shortcut manipulation)
- pygetwindow (Big Picture window detection)
- pystray (Tray icon)
- pyinstaller (Standalone exe creation)

`pip install pyqt5 winshell pygetwindow pystray pyinstaller`

## Build

Make sure you installed required dependencies.<br/>

- Clone this repository `git clone git@github.com:Odizinne/BigPictureTV.git`.<br/>
- Move inside the directory `cd BigPictureTV`.<br/>
- Build the executable `pyinstaller BigPictureTV.spec`.

Since we're building with `console=False`, windows defender may not like it. You'll need to authorize the file.

## Todo

- code cleanup