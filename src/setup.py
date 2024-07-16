import os
from cx_Freeze import setup, Executable
from setuptools.command.install import install as _install
import winshell

src_dir = os.path.dirname(os.path.abspath(__file__))
build_dir = "build/BigPictureTV"
install_dir = os.path.join(os.getenv('LOCALAPPDATA'), 'programs', 'BigPictureTV')

include_files = [
    os.path.join(src_dir, "icons")
]

zip_include_packages = ['PyQt6', 'winshell', 'PyGetWindow', 'pywin32']

build_exe_options = {
    "include_files": include_files,
    "build_exe": build_dir,
    "zip_include_packages": zip_include_packages,   
}

executables = [
    Executable(
        script = os.path.join(src_dir, 'BigPictureTV.py'), 
        base = "Win32GUI", 
        icon = os.path.join(src_dir, "icons/steamos-logo.ico"),
        target_name = "BigpictureTV"
    )
]

class InstallCommand(_install):
    def run(self):
        if not os.path.exists(install_dir):
            os.makedirs(install_dir)
        if not os.path.exists(build_dir):
            print("##################################################")
            print("# Nothing to install.                            #")
            print("# Please build the project first.                #")
            print("##################################################")
            return
        self.copy_tree(build_dir, install_dir)
        print("")
        print(f"Executable installed to {install_dir}")

        shortcut_path = os.path.join(winshell.programs(), "BigPictureTV.lnk")
        target = os.path.join(install_dir, "BigPictureTV.exe")
        icon = os.path.join(src_dir, "icons/steamos-logo.ico")

        winshell.CreateShortcut(
            Path=shortcut_path,
            Target=target,
            Icon=(icon, 0),
            Description="BigPictureTV",
            StartIn=install_dir
        )

        print("Created shortcut in start menu")

setup(
    name='BigPictureTV',
    version='1.0',
    options={"build_exe": build_exe_options},
    executables=executables,
    cmdclass={'install': InstallCommand}
)
