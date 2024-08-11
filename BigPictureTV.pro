QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Optional: Uncomment to disable deprecated APIs before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    src/audiomanager.cpp \
    src/colorutils.cpp \
    src/main.cpp \
    src/bigpicturetv.cpp \
    src/shortcutmanager.cpp \
    src/steamwindowmanager.cpp \
    src/utils.cpp

HEADERS += \
    src/audiomanager.h \
    src/bigpicturetv.h \
    src/colorutils.h \
    src/shortcutmanager.h \
    src/steamwindowmanager.h \
    src/utils.h

FORMS += \
    src/bigpicturetv.ui

# Add the necessary libraries for COM functionality, Shell operations, and other Windows API functions
LIBS += -lole32 -lshell32 -luser32 -ladvapi32

RESOURCES += src/resources.qrc
RC_FILE = src/appicon.rc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
