Python script to automate switching from monitor to TV when starting Steam Big Picture.<br/>
Also switch audio output.

I use clone instead of internal so the HDMI interface is not disabled, preventing using a sleep to wait from HDMI turning on.

- `pip install pygetwindow` (handle window title)
- `Install-Module -Name AudioDeviceCmdlets` (handle audio switching)

Reset screens / audio at start.

Clone this repo, edit the file to suit your needs, and then `pip install pyinstaller`

`pyinstaller --onefile --noconsole bigpicture-external.py`

You can then create a shortcut and place it to shell:startup.

Windows defender will block the generated exe file. You may whitelist it.
