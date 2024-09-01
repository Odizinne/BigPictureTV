QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 \
          silent \

# Optional: Uncomment to disable deprecated APIs before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# Fetch git information for about window
GIT_COMMIT_ID = $$system(git log -n 1 --pretty=format:"%H")
GIT_COMMIT_DATE = $$system(git log -n 1 --pretty=format:"%ci")
GIT_BRANCH = $$system(git branch --show-current)

DEFINES += \
    GIT_COMMIT_ID=\"\"\"$$GIT_COMMIT_ID\"\"\" \
    GIT_COMMIT_DATE=\"\"\"$$GIT_COMMIT_DATE\"\"\"

INCLUDEPATH += \
    src/AudioManager\
    src/BigPictureTV \
    src/Configurator \
    src/NightLightSwitcher \
    src/ShortcutManager \
    src/SteamWindowManager \
    src/Utils \

SOURCES += \
    src/AudioManager/audiomanager.cpp \
    src/BigPictureTV/BigPictureTV.cpp \
    src/main.cpp \
    src/NightLightSwitcher/NightLightSwitcher.cpp \
    src/Configurator/configurator.cpp \
    src/ShortcutManager/shortcutmanager.cpp \
    src/SteamWindowManager/steamwindowmanager.cpp \
    src/Utils/utils.cpp

HEADERS += \
    src/AudioManager/audiomanager.h \
    src/BigPictureTV/BigPictureTV.h \
    src/Configurator/configurator.h \
    src/NightLightSwitcher/NightLightSwitcher.h \
    src/ShortcutManager/shortcutmanager.h \
    src/SteamWindowManager/steamwindowmanager.h \
    src/Utils/utils.h

FORMS += \
    src/Configurator/configurator.ui

TRANSLATIONS += \
    src/Resources/Tr/BigPictureTV_fr.ts \
    src/Resources/Tr/BigPictureTV_en.ts

RESOURCES += \
    src/Resources/resources.qrc \
    src/Resources/translations.qrc

RC_FILE = src/Resources/appicon.rc

LIBS += -lole32 -luser32 -ladvapi32 -lshell32

DEPENDENCIES_DIR = $$PWD/dependencies
DEST_DIR = $$OUT_PWD/release/dependencies

QMAKE_POST_LINK += powershell -Command "New-Item -ItemType Directory -Path '$$DEST_DIR' -Force; Copy-Item -Path '$$DEPENDENCIES_DIR\*' -Destination '$$DEST_DIR' -Recurse -Force"

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

