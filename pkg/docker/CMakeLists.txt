cmake_minimum_required(VERSION 3.31)

project(external_libs)

include(ExternalProject)
include(ProcessorCount)

ProcessorCount(NPROC)

set(QT_V 6.10)
set(QT_VV 6.10.1)
set(FFMPEG_V 8.0.1)
set(INSTALL_PREFIX /usr)

set(QT_CONF
    ./configure
    -prefix ${INSTALL_PREFIX}
    -ccache
    -opensource
    -confirm-license

    # Build options:
    -release
    -static
    -dbus-linked

    -nomake examples
    -nomake tests
    -nomake benchmarks
    -nomake manual-tests
    -nomake minimal-static-tests

    # Core options:
    -qt-doubleconversion
    -no-glib
    -no-inotify
    -no-icu
    -no-jemalloc
    -qt-pcre
    -qt-zlib

    # Network options:
    -ssl -openssl-linked
    -no-system-proxies

    #Gui, printing, widget options:
    -no-cups
    -no-fontconfig
    -qt-freetype
    -qt-harfbuzz
    -no-opengles3
    -qpa wayland
    -default-qpa wayland
    -no-xcb-xlib

    # Platform backends:
    -no-directfb
    -no-eglfs
    -no-gbm
    -no-kms
    -no-linuxfb

    # Input backends:
    -no-libudev
    -no-evdev
    -no-libinput
    -no-mtdev
    -no-tslib
    -xkbcommon

    # Image formats:
    -no-gif
    -no-ico
    -qt-libpng
    -qt-libjpeg

    # Database options:
    -qt-sqlite
)

# FFmpeg
ExternalProject_Add(
ffmpeg
URL https://www.ffmpeg.org/releases/ffmpeg-${FFMPEG_V}.tar.xz
URL_HASH SHA256=05ee0b03119b45c0bdb4df654b96802e909e0a752f72e4fe3794f487229e5a41
BUILD_IN_SOURCE 1
CONFIGURE_COMMAND ./configure --prefix=${INSTALL_PREFIX}
BUILD_COMMAND make -j${NPROC}
INSTALL_COMMAND make install
)

add_definitions('QT_QPA_PLATFORMS=wayland\;xcb')

# Qt
ExternalProject_Add(
qtbase
URL https://download.qt.io/archive/qt/${QT_V}/${QT_VV}/submodules/qtbase-everywhere-src-${QT_VV}.tar.xz
URL_HASH SHA256=5a6226f7e23db51fdc3223121eba53f3f5447cf0cc4d6cb82a3a2df7a65d265d
BUILD_IN_SOURCE 1
CONFIGURE_COMMAND ${QT_CONF}
BUILD_COMMAND cmake --build . --parallel --config Release
INSTALL_COMMAND cmake --install .
DEPENDS ffmpeg
)

set(QT_SRC
    # Module            Depnds on       SHA256
    qtshadertools       qtbase          b67894a63352b53dad0d46f5300c62b8cd6783db575492d6b25d4fdc9af55bb6
    qtdeclarative       qtshadertools   4fb4efb894e0b96288543505d69794d684bcfbe4940ce181d3e6817bda54843e
    qtmultimedia        qtdeclarative   f7a4f9bc2840d4f0f9f7329f0dcb3d3500c54177b8e368091a3727c7320e67b8
    qtpositioning       qtdeclarative   abb311ef1bd6e39f090d22480e265d13f8537d2e2f4c88f22d6519547f46be23
    qtlocation          qtpositioning   2b9618f1b99cdc51d6697a79f3a57f4c2b629711549912067db8cd2b40794f09
    qtwayland           qtdeclarative   49bf6db800227a6b2c971f4c5d03dd1e81297e7ffb296ce4a96437304f27cb13
    qtsvg               qtdeclarative   c02f355a58f3bbcf404a628bf488b6aeb2d84a94c269afdb86f6e529343ab01f
    qtvirtualkeyboard   qtsvg           5b9cde3188afbc01b602b9016cee95ccd536aea43a6e6cfd297b44f328b9b6df
)

while(QT_SRC)

    list(POP_FRONT QT_SRC QT_LIB_NAME DEPENDS_ON QT_LIB_HASH)

    ExternalProject_Add(
        ${QT_LIB_NAME}
        URL https://download.qt.io/archive/qt/${QT_V}/${QT_VV}/submodules/${QT_LIB_NAME}-everywhere-src-${QT_VV}.tar.xz
        URL_HASH SHA256=${QT_LIB_HASH}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND cmake . -GNinja "-DCMAKE_INSTALL_PREFIX:STRING=${INSTALL_PREFIX}" -DCMAKE_BUILD_TYPE=Release
        BUILD_COMMAND cmake --build . --parallel --config Release
        INSTALL_COMMAND cmake --install .
        DEPENDS ${DEPENDS_ON}
    )
endwhile()
