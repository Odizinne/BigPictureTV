# BigPictureTV

This Python project automates switching between a monitor and a TV when launching Steam Big Picture on Windows.  
*Looking for the [linux version](https://github.com/Odizinne/BigpictureTV-Linux)?*

**If you appreciate my work and would like to support me:**

<a href="https://ko-fi.com/odizinne">
  <img src="assets/kofi_button_red.png" alt="Ko-fi" width="200" height="auto">
</a>

## Overview

![image](assets/screenshot.png)

## Requirements

**Runtime:**
- [AudioDeviceCmdlets](https://github.com/frgnca/AudioDeviceCmdlets) for audio output switching.
 
## Download

Head to [release](https://github.com/Odizinne/BigPictureTV/releases/latest) section and grab the latest one.

Place the directory wherever you like (`%localappdata%\Programs` is a good one)

## Usage


If you have a two monitors setup (Main monitor + TV), i do recommend you uncheck `Do not use displayswitch`.  
displayswitch.exe will automatically disable your primary monitor and enable your external monitor.

If you have more than two monitors, you'll need to check it, and select your preferred monitor in Steam Big Picture settings.  

Specify your audio outputs.  
You can use a short name. BigPictureTV will try to find the correct audio output from keywords. Less is more.

**You're all set!** You can now close the settings window.

## Build

- Install Python.
- Clone this repository: `git clone https://github.com/Odizinne/BigPictureTV.git`<br/>
- CD inside the cloned directory: `cd BigPictureTV`<br/>
- Install dependencies: `pip install -r requirements.txt`
- Build the executable: `python src\setup.py build`<br/>

## I found a bug

Feel free to [open an issue](https://github.com/Odizinne/BigPictureTV/issues/new) in the repository.  
Be sure to add as many information as possible to help me reproduce it!