# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['BigPictureTV.py'],
    pathex=[],
    binaries=[],
    datas=[('steamos-logo.png', '.'), ('design.ui', '.'), ('help.ui', '.'), ('steamos-logo-light.png', '.'), ('steamos-logo-dark.png', '.')],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='BigPictureTV',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=['steamos-logo.png'],
)
