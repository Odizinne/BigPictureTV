# BigPictureTV

This Python project automates switching between a monitor and a TV when launching Steam Big Picture on Windows.

⚠️This application is designed for dual screen setup (monitor + TV). It might not work with more than 2 screens⚠️  

![image](https://github.com/user-attachments/assets/890e2c2a-5302-4ee3-b212-a2ba0dbe8a39)

## Download

Head to [release](https://odizinne.net/Odizinne/BigPictureTV/releases) section and grab the latest one.

Place the directory wherever you like (`%localappdata%\Programs` is a good one)

## Requirements

**Runtime:**
- [AudioDeviceCmdlets](https://github.com/frgnca/AudioDeviceCmdlets) for audio output switching.
 
## Build

- Install Python.
- Clone this repository: `git clone git@github.com:Odizinne/BigPictureTV.git`<br/>
- CD inside the cloned directory: `cd BigPictureTV`<br/>
- Install dependencies: `pip install -r requirements.txt`
- Build the executable: `python src\setup.py build`<br/>
- Install directory and create startup shortcut: `python .\src\setup.py install`
