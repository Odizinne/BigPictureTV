QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Optional: Uncomment to disable deprecated APIs before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    audiomanager.cpp \
    colorutils.cpp \
    main.cpp \
    bigpicturetv.cpp \
    shortcutmanager.cpp \
    steamwindowmanager.cpp \
    utils.cpp

HEADERS += \
    audiomanager.h \
    bigpicturetv.h \
    colorutils.h \
    shortcutmanager.h \
    steamwindowmanager.h \
    utils.h

FORMS += \
    bigpicturetv.ui

# Add the necessary libraries for COM functionality, Shell operations, and other Windows API functions
LIBS += -lole32 -lshell32 -luser32 -ladvapi32

RESOURCES += resources.qrc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
