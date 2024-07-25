# BigPictureTV

This Python project automates switching between a monitor and a TV when launching Steam Big Picture on Windows.  
*Looking for the [linux version](https://github.com/Odizinne/BigpictureTV-Linux)?*

![image](assets/screenshot.png)

## Requirements

**Runtime:**
- [AudioDeviceCmdlets](https://github.com/frgnca/AudioDeviceCmdlets) for audio output switching.
 
## Download

Head to [release](https://github.com/Odizinne/BigPictureTV/releases/latest) section and grab the latest one.

Place the directory wherever you like (`%localappdata%\Programs` is a good one)

## Usage

Specify your audio and video outputs for both modes.  
If you have a two monitors setup (Main monitor + TV), i do recommend you keep `legacy screen control` checked and do not bother with manual screen select.

For audio, you can use a short name. BigPictureTV will try to find the correct audio output from keywords. Less is more.

You're all set! Close the settings window, BigPictureTV will run in background and switch audio and video to the one you choosed when Big Picture window is found.

## Build

- Install Python.
- Clone this repository: `git clone git@github.com:Odizinne/BigPictureTV.git`<br/>
- CD inside the cloned directory: `cd BigPictureTV`<br/>
- Install dependencies: `pip install -r requirements.txt`
- Build the executable: `python src\setup.py build`<br/>
- Install directory and create startup shortcut: `python .\src\setup.py install`

## To-do
- ~~Autodetect steam language~~

