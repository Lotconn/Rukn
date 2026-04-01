cmake_minimum_required(VERSION 3.30)

project(external_libs)

include(ExternalProject)
include(ProcessorCount)

ProcessorCount(NPROC)

set(QT_V 6.11)
set(QT_VV 6.11.0)
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
URL_HASH SHA256=231ad85979864d914dc9568a1b71c91d6cf20d7b2021d059103bf0eb51cb755e
BUILD_IN_SOURCE 1
CONFIGURE_COMMAND ${QT_CONF}
BUILD_COMMAND cmake --build . --parallel --config Release
INSTALL_COMMAND cmake --install .
DEPENDS ffmpeg
)

set(QT_SRC
    # Module            Depnds on       SHA256
    qtshadertools       qtbase          e43cb1ae8809b2a858281ee269f98da59d0fc1bcf958ca5510c81f7ad3d2e14a
    qtdeclarative       qtshadertools   4eece569431ddf8324e7d322fa27001916570b23df535f8fb28aba445eedfde9
    qtpositioning       qtdeclarative   d61fd0985ede513ec34d2d1c1e92f383eb8eb46678ca9da805cf795cccb796e9
    qtlocation          qtpositioning   89b8386a8ae9e0b40a43fad398ac344f93a3b0d22f09bec4631f25d79135abef
    qtmultimedia        qtdeclarative   90c4cac0a7a983b68d1b0873d0714e7873b9a493403fa8593e8a4eea3ea26040


    qtsvg               qtdeclarative   dfa8d653be07087d9407ed4a4ebae847f8953e0b7abd829f089803ab652a30e6
    qtvirtualkeyboard   qtsvg           d88a4b1713a313e3ac06c32837b5d00724d1dcf7b44c2594f1029f7c74a8e686
    qtwayland           qtdeclarative   e710e6e760f92922b86e4dd68f6bbe94ef6510919519d1b0068e874b5ad84d37
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
