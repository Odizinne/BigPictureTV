from cx_Freeze import setup, Executable

build_dir = "build/BigPictureTV"
base = "Win32GUI"
system_icon = "src/icons/steamos-logo.ico"

zip_include_packages = ['PyQt6', 'winshell', 'PyGetWindow']

build_exe_options = {
    "include_files": [('src/icons/steamos-logo.png', 'src/icons/steamos-logo.png')],
    "build_exe": build_dir,
    "zip_include_packages": zip_include_packages,   
}

executables = [
    Executable('src/BigPictureTV.py', base=base, icon=system_icon)
]

setup(
    name='BigPictureTV',
    version='1.0',
    options={"build_exe": build_exe_options},
    executables=executables
)
