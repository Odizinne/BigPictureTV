# BigPictureTV

This Python project automates switching between a monitor and a TV when launching Steam Big Picture on Windows.

![image](https://github.com/user-attachments/assets/890e2c2a-5302-4ee3-b212-a2ba0dbe8a39)

## Download

Head to [release](https://github.com/Odizinne/BigPictureTV/releases) section and grab the latest one.

Place the directory wherever you like (`%localappdata%\Programs` is a good one)

## Requirements

**Runtime:**
- [AudioDeviceCmdlets](https://github.com/frgnca/AudioDeviceCmdlets) for audio output switching.
 
**Build dependencies:**
- Python
- PyQt6 (Ui)
- winshell (Startup shortcut manipulation)
- pygetwindow (Big Picture window detection)
- cx_freeze (Standalone exe creation)

`pip install pyqt6 winshell pygetwindow cx_freeze`

## Build

Make sure you installed required dependencies.<br/>

- Clone this repository `git clone git@github.com:Odizinne/BigPictureTV.git`.<br/>
- Move inside the source directory `cd BigPictureTV\src`.<br/>
- Build the executable `python setup.py build`.

## Todo

- code cleanup
