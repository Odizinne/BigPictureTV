QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 silent lrelease embed_translations

QM_FILES_RESOURCE_PREFIX = /translations
DEPENDENCIES_DIR = $$PWD/dependencies
DEST_DIR = $$OUT_PWD/release/dependencies

INCLUDEPATH +=                                                  \
    src/AudioManager                                            \
    src/BigPictureTV                                            \
    src/Configurator                                            \
    src/NightLightSwitcher                                      \
    src/ShortcutManager                                         \
    src/SteamWindowManager                                      \
    src/Utils                                                   \

SOURCES += \
    src/AudioManager/AudioManager.cpp                           \
    src/BigPictureTV/BigPictureTV.cpp                           \
    src/NightLightSwitcher/NightLightSwitcher.cpp               \
    src/Configurator/Configurator.cpp                           \
    src/ShortcutManager/ShortcutManager.cpp                     \
    src/SteamWindowManager/SteamwindowManager.cpp               \
    src/Utils/utils.cpp                                         \
    src/main.cpp                                                \

HEADERS +=                                                      \
    src/AudioManager/AudioManager.h                             \
    src/BigPictureTV/BigPictureTV.h                             \
    src/Configurator/Configurator.h                             \
    src/NightLightSwitcher/NightLightSwitcher.h                 \
    src/ShortcutManager/ShortcutManager.h                       \
    src/SteamWindowManager/SteamwindowManager.h                 \
    src/Utils/utils.h                                           \

FORMS +=                                                        \
    src/Configurator/Configurator.ui                            \

TRANSLATIONS +=                                                 \
    src/Resources/Tr/BigPictureTV_fr.ts                         \
    src/Resources/Tr/BigPictureTV_en.ts                         \

RESOURCES +=                                                    \
    src/Resources/resources.qrc                                 \

RC_FILE = src/Resources/appicon.rc

LIBS += -lole32 -luser32 -ladvapi32 -lshell32

QMAKE_POST_LINK += powershell -Command "New-Item -ItemType Directory -Path '$$DEST_DIR' -Force; Copy-Item -Path '$$DEPENDENCIES_DIR\*' -Destination '$$DEST_DIR' -Recurse -Force"
