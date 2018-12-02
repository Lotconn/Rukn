QML_IMPORT_PATH = ./

CONFIG += qt c++17 qtquickcompiler
DEFINES -= ARABIC
#DEFINES += QT_NO_DEBUG_OUTPUT

win32 || mac || linux {
QT += core gui network qml quick quickcontrols2 multimedia location positioning
DEFINES += DESKTOP
TARGET = munadi
}

CONFIG(flatpak) {
message("flatpak build")

target.path = $$(FLATPAK_DEST)/bin

desktop.files = xdg/org.munadi.Munadi.desktop
desktop.path = $$(FLATPAK_DEST)/share/applications

appdata.files = xdg/org.munadi.Munadi.appdata.xml
appdata.path = $$(FLATPAK_DEST)/share/appdata

icons.files = xdg/org.munadi.Munadi.icons/*
icons.path = $$(FLATPAK_DEST)/share/icons

INSTALLS += target desktop appdata icons
}

android {
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
QT += core gui network qml quick quickcontrols2 multimedia location positioning
DEFINES += ANDROID
TARGET = Munadi
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

DISTFILES += qml/*.qml \
    android/AndroidManifest.xml \

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../libs/android/openssl/libcrypto.so \
        $$PWD/../libs/android/openssl/libssl.so
}

