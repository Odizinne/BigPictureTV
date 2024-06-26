# BigPictureTV

This Python project automates switching between a monitor and a TV when launching Steam Big Picture on Windows.

![image](https://github.com/Odizinne/bigpicturetv/assets/102679854/90c55be3-dbe8-4635-b655-0c831351e338)

![image](https://github.com/Odizinne/BigPictureTV/assets/102679854/384e1369-ce2f-4178-b2bf-023a8e7d709c)

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

- AudioDeviceCmdlets install prompt
