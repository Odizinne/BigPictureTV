QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 silent lrelease embed_translations

QM_FILES_RESOURCE_PREFIX = /translations

INCLUDEPATH +=                                              \
    AudioManager/                                           \
    BigPictureTV/                                           \
    Configurator/                                           \
    NightLightSwitcher/                                     \
    ShortcutManager/                                        \
    SteamWindowManager/                                     \
    Utils/                                                  \
    Dependencies/HDRtray/common/                            \

SOURCES +=                                                  \
    AudioManager/AudioManager.cpp                           \
    BigPictureTV/BigPictureTV.cpp                           \
    NightLightSwitcher/NightLightSwitcher.cpp               \
    Configurator/Configurator.cpp                           \
    ShortcutManager/ShortcutManager.cpp                     \
    SteamWindowManager/SteamwindowManager.cpp               \
    Utils/Utils.cpp                                         \
    main.cpp                                                \
    Dependencies/HDRtray/common/hdr.cpp                     \

HEADERS +=                                                  \
    AudioManager/AudioManager.h                             \
    AudioManager/PolicyConfig.h                             \
    BigPictureTV/BigPictureTV.h                             \
    Configurator/Configurator.h                             \
    NightLightSwitcher/NightLightSwitcher.h                 \
    ShortcutManager/ShortcutManager.h                       \
    SteamWindowManager/SteamwindowManager.h                 \
    Utils/Utils.h                                           \
    Dependencies/HDRtray/common/hdr.h                       \

FORMS +=                                                    \
    Configurator/Configurator.ui                            \

TRANSLATIONS +=                                             \
    Resources/Tr/BigPictureTV_fr.ts                         \
    Resources/Tr/BigPictureTV_en.ts                         \

RESOURCES +=                                                \
    Resources/resources.qrc                                 \

RC_FILE = Resources/appicon.rc

LIBS += -lole32 -luser32 -ladvapi32 -lshell32
