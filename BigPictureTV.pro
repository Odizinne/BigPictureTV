QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Optional: Uncomment to disable deprecated APIs before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += \
    src/AudioManager \
    src/BigPictureTV \
    src/ColorUtils \
    src/ShortcutManager \
    src/SteamWindowManager \
    src/Utils

SOURCES += \
    src/AudioManager/audiomanager.cpp \
    src/ColorUtils/colorutils.cpp \
    src/main.cpp \
    src/BigPictureTV/bigpicturetv.cpp \
    src/ShortcutManager/shortcutmanager.cpp \
    src/SteamWindowManager/steamwindowmanager.cpp \
    src/Utils/utils.cpp

HEADERS += \
    src/AudioManager/audiomanager.h \
    src/BigPictureTV/bigpicturetv.h \
    src/ColorUtils/colorutils.h \
    src/ShortcutManager/shortcutmanager.h \
    src/SteamWindowManager/steamwindowmanager.h \
    src/Utils/utils.h

FORMS += \
    src/BigPictureTV/bigpicturetv.ui

TRANSLATIONS += \
    src/Resources/Tr/BigPictureTV_fr.ts \
    src/Resources/Tr/BigPictureTV_en.ts

RESOURCES += \
    src/Resources/resources.qrc \
    src/Resources/translations.qrc

RC_FILE = src/Resources/appicon.rc

LIBS += -lole32 -luser32 -ladvapi32 -lshell32

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

