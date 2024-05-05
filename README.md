This Python project automates switching between a monitor and a TV when launching Steam Big Picture and also handles audio output switching.

# Requirements
- Python `pip install pyinstaller pywin32 pygetwindow psutil winshell`
- PowerShell `Install-Module -Name AudioDeviceCmdlets`

# Usage
The first time you launch it, a window will show.<br/>
Specify your audio output settings for both desktop and gamemode.<br/>
Specify your Steam Big Picture window title.<br/>
run with `--config` to open this window again if needed.

If you configure this script to auto-start, you can safely shut down your PC with Steam Big Picture active, as it will reset screens and audio settings upon startup.
