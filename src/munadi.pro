QML_IMPORT_PATH = ./

CONFIG += qt c++17 qtquickcompiler
DEFINES -= ARABIC
#DEFINES += QT_NO_DEBUG_OUTPUT

win32 || mac || linux {
QT += core gui network qml quick quickcontrols2 multimedia location positioning
DEFINES += DESKTOP
TARGET = munadi
}

target.path = /usr/bin
desktop.files = xdg/org.munadi.Munadi.desktop
desktop.path = /usr/share/applications
metainfo.files = xdg/org.munadi.Munadi.metainfo.xml
metainfo.path = /usr/share/metainfo
icons.files = xdg/org.munadi.Munadi.icons/*
icons.path = /usr/share/icons

CONFIG(flatpak) {
message("flatpak build")
target.path = $$(FLATPAK_DEST)/bin
desktop.path = $$(FLATPAK_DEST)/share/applications
metainfo.path = $$(FLATPAK_DEST)/share/metainfo
icons.path = $$(FLATPAK_DEST)/share/icons
}

INSTALLS += target desktop metainfo icons

android {
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
QT += core gui network qml quick quickcontrols2 multimedia location positioning
DEFINES += ANDROID
TARGET = Munadi

DISTFILES += qml/*.qml \
    android/AndroidManifest.xml \

include($$PWD/../libs/android/openssl/openssl.pri)

}

HEADERS +=                  \
    cpp/updater.h           \
    cpp/libitl/hijri.h      \
    cpp/engine.h

SOURCES += cpp/main.cpp     \
    cpp/updater.cpp         \
    cpp/libitl/hijri.c      \
    cpp/engine.cpp

RESOURCES += \
    rc.qrc

lupdate_only{
SOURCES = qml/*.qml
}

DISTFILES += \
    qml/*.qml
