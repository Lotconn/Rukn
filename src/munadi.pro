QML_IMPORT_PATH = ./

CONFIG += qt c++17 qtquickcompiler
DEFINES -= ARABIC
#DEFINES += QT_NO_DEBUG_OUTPUT

win32 || mac || linux {
QT += core gui network qml quick multimedia location positioning
DEFINES += DESKTOP
}

android {
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
QT += core gui network qml quick multimedia location positioning
DEFINES -= DESKTOP
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
    android_release.keystore \
    android/assets/audio/athan.ogg \
    android/res/drawable-ldpi/icon.png \
    munadi64.png \
    munadi80.png

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../libs/android/openssl/libcrypto.so \
        $$PWD/../libs/android/openssl/libssl.so
}

