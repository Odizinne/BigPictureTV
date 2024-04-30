This Python script automates switching between a monitor and a TV when launching Steam Big Picture and also handles audio output switching.

Requirements:
- Download [py-audiodevices](https://github.com/Odizinne/py-audiodevices) and place it in the same directory to manage audio output switching.
- Install pygetwindow via pip (`pip install pygetwindow`) to handle window titles.

Usage:
1. Launch the script once to create the configuration file.
2. Specify your audio output settings for both desktop and gamemode in the config file.
   - Use the same names as they appear in Windows for the audio outputs.

If you configure this script to auto-start, you can safely shut down your PC with Steam Big Picture active, as it will reset screens and audio settings upon startup.
