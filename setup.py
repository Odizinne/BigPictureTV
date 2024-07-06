from cx_Freeze import setup, Executable

build_dir = "build/BigPictureTV"
base = "Win32GUI"
system_icon = "icons/steamos-logo.ico"

zip_include_packages = ['PyQt6', 'winshell', 'PyGetWindow', 'pywin32']

build_exe_options = {
    "include_files": [('icons/steamos-logo.png', 'icons/steamos-logo.png')],
    "build_exe": build_dir,
    "zip_include_packages": zip_include_packages,   
}

executables = [
    Executable('BigPictureTV.py', base=base, icon=system_icon)
]

setup(
    name='BigPictureTV',
    version='1.0',
    options={"build_exe": build_exe_options},
    executables=executables
)
