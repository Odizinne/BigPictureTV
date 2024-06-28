# BigPictureTV

This Python project automates switching between a monitor and a TV when launching Steam Big Picture on Windows.

![image](https://github.com/Odizinne/BigPictureTV/assets/102679854/1ed0abac-47e0-4a83-acd8-2735590bfbc5)

![image](https://github.com/Odizinne/BigPictureTV/assets/102679854/d69a2bd6-aa1f-44ea-adef-dc88c0d2a242)

## Download

Head to [release](https://github.com/Odizinne/BigPictureTV/releases) section and grab the latest one.

if you're feeling adventurous you can download the latest main build [here](https://raw.githubusercontent.com/Odizinne/BigPictureTV/main/build/BigPictureTV.zip)<br/>
Main build is updated often, but may contain bugs.
## Requirements

**Runtime:**
- [AudioDeviceCmdlets](https://github.com/frgnca/AudioDeviceCmdlets) for audio output switching.
 
**Build dependencies:**
- Python
- PyQt5 (Ui)
- winshell (Startup shortcut manipulation)
- pygetwindow (Big Picture window detection)
- cx_freeze (Standalone exe creation)

`pip install pyqt5 winshell pygetwindow cx_freeze`

## Build

Make sure you installed required dependencies.<br/>

- Clone this repository `git clone git@github.com:Odizinne/BigPictureTV.git`.<br/>
- Move inside the directory `cd BigPictureTV`.<br/>
- Build the executable `python setup_cxfreeze.py build`.

## Todo

- code cleanup
