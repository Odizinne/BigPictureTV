import subprocess

DISPLAYSWITCH = "DisplaySwitch.exe"
CLONE = "/clone"
INTERNAL = "/internal"
EXTERNAL = "/external"
EXTEND = "/extend"


def enable_clone_mode():
    subprocess.run([DISPLAYSWITCH, CLONE])


def enable_external_mode():
    subprocess.run([DISPLAYSWITCH, EXTERNAL])


def enable_internal_mode():
    subprocess.run([DISPLAYSWITCH, INTERNAL])


def enable_extend_mode():
    subprocess.run([DISPLAYSWITCH, EXTEND])
