import os
from cx_Freeze import setup, Executable

src_dir = os.path.dirname(os.path.abspath(__file__))
build_dir = "build/BigPictureTV"

include_files = [
    os.path.join(src_dir, "icons"),
    os.path.join(src_dir, "tr"),
]

zip_include_packages = ["PyQt6"]

build_exe_options = {
    "include_files": include_files,
    "build_exe": build_dir,
    "zip_include_packages": zip_include_packages,
    "excludes": ["tkinter"],
    "silent": True,
    "include_msvcr": True,
}

executables = [
    Executable(
        script=os.path.join(src_dir, "bigpicturetv.py"),
        base="Win32GUI",
        icon=os.path.join(src_dir, "icons/icon.ico"),
        target_name="BigpictureTV",
    )
]

setup(
    name="BigPictureTV",
    version="1.0",
    options={"build_exe": build_exe_options},
    executables=executables,
)
