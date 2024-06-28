from cx_Freeze import setup, Executable

setup(
    name='BigPictureTV',
    version='1.0',
    description='BigPictureTV application',
    executables=[
        Executable('src/BigPictureTV.py', base='Win32GUI', icon='src/icons/steamos-logo.ico', target_name='BigPictureTV')
    ],
    options={
        'build_exe': {
            'include_files': [
                ('src/ui/design.ui', 'ui/design.ui'),
                ('src/ui/help.ui', 'ui/help.ui'),
                ('src/ui/style.qss', 'ui/style.qss'),
                ('src/icons/steamos-logo.png', 'icons/steamos-logo.png')
            ],
            'excludes': [],
            'optimize': 0,
            'build_exe': 'build/BigPictureTV',
        }
    }
)
