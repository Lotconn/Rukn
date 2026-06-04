cmake_minimum_required(VERSION 3.30...4.3)

project(external_libs)

include(ExternalProject)
include(ProcessorCount)

ProcessorCount(NPROC)

set(QT_V 6.11)
set(QT_VV 6.11.1)
set(FFMPEG_V 7.1.3)
set(INSTALL_PREFIX /usr)

set(COMMON_FLAGS
    "-DCMAKE_CXX_FLAGS=' -O3 -pipe -Wl,-z,relro,-z,now -Wl,--as-needed -Wp,-D_FORTIFY_SOURCE=3 -fstack-protector-strong -fcf-protection -fstack-clash-protection '"
)

set(FFMPEG_CONF
    ./configure

    # Standard options:
    --prefix=${INSTALL_PREFIX}

    # Configuration options:
    --disable-all

    # Component options:
    --enable-avcodec
    --enable-avformat
    --enable-swresample
    --enable-swscale

    # Individual component options:
    --enable-decoder=aac,ac3,alac,eac3,amrnb,amrwb,flac,mp3,opus,vorbis
    --enable-decoder=pcm_s16le,pcm_s24le,pcm_s32le,pcm_f32le,pcm_u8,pcm_alaw,pcm_mulaw

    --enable-demuxer=aac,ac3,alac,amr,asf,au,flac,matroska,mov
    --enable-demuxer=mp3,mpegps,mpegts,ogg,wav

    --disable-v4l2-m2m

    --enable-parser=aac,aac_latm,ac3,amr,flac,mpegaudio,opus,vorbis

    --enable-protocol=cache,fd,file,pipe,unix

    # External library support:
    --disable-iconv

    # Toolchain options:
    --enable-lto

    # Advanced options:
    --extra-cxxflags=" -O3 -pipe -Wp,-D_FORTIFY_SOURCE=3 -fstack-protector-strong -fcf-protection -fstack-clash-protection "
    --extra-ldflags=" -Wl,-z,relro,-z,now -Wl,--as-needed "

    # Developer options:
    --disable-debug
)

set(QT_CONF
    ./configure
    -prefix ${INSTALL_PREFIX}
    -opensource
    -confirm-license

    # Build options:
    -dbus-linked

    -release
    -static
    -no-sbom
    -reduce-exports
    -reduce-relocations
    -ltcg
    -glibc-fortify-source
    -stack-protector
    -stack-clash-protection
    -relro-now-linker
    -ccache

    # Build environment
    QT_BUILD_TOOLS_BY_DEFAULT=OFF
    "-DCMAKE_CXX_FLAGS=' -O3 -pipe '"

    # Component selection:
    -nomake examples
    -nomake tests
    -nomake benchmarks
    -nomake manual-tests
    -nomake minimal-static-tests
    -nomake doc-snippets

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

    # Gui, printing, widget options:
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
    -no-xcb

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

# Flags passed to every sub-module cmake invocation.
set(QT_SUBMODULE_CMAKE_ARGS
    ${COMMON_FLAGS}
    "-DCMAKE_INSTALL_PREFIX:STRING=${INSTALL_PREFIX}"
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_C_COMPILER_LAUNCHER=ccache
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE=ON
    -DQT_BUILD_BENCHMARKS=OFF
    -DQT_BUILD_DOC_SNIPPETS=FALSE
    -DQT_BUILD_EXAMPLES=OFF
    -DQT_BUILD_MANUAL_TESTS=OFF
    -DQT_BUILD_MINIMAL_STATIC_TESTS=FALSE
    -DQT_BUILD_TESTS=OFF
    -DQT_BUILD_TOOLS_BY_DEFAULT=OFF
    -DQT_GENERATE_SBOM=OFF
    -DQT_QPA_DEFAULT_PLATFORM=wayland
    -DQT_QPA_PLATFORMS=wayland
    -DQT_USE_CCACHE=ON
    -GNinja
)

# FFmpeg
ExternalProject_Add(ffmpeg
    URL https://www.ffmpeg.org/releases/ffmpeg-${FFMPEG_V}.tar.xz
    URL_HASH SHA256=f0bf043299db9e3caacb435a712fc541fbb07df613c4b893e8b77e67baf3adbe
    BUILD_IN_SOURCE ON
    CONFIGURE_COMMAND ${FFMPEG_CONF}
    BUILD_COMMAND make -j${NPROC}
    INSTALL_COMMAND make install
)

add_definitions('QT_QPA_PLATFORMS=wayland\;xcb')

# Qt
ExternalProject_Add(qtbase
    URL https://download.qt.io/archive/qt/${QT_V}/${QT_VV}/submodules/qtbase-everywhere-src-${QT_VV}.tar.xz
    URL_HASH SHA256=d9594a31228aa23ad6b531719a29b45f0f3989fe6c136d45767ea179f233c1ac
    BUILD_IN_SOURCE ON
    CONFIGURE_COMMAND ${QT_CONF}
    BUILD_COMMAND cmake --build . --parallel --config Release
    INSTALL_COMMAND cmake --install .
    DEPENDS ffmpeg
)

set(QT_SRC
    # Module            Depnds on       SHA256
    qtshadertools       qtbase          2075052f9b23bcf9de045bbd180037084942f82cce870aab14a1454902c982fc
    qtdeclarative       qtshadertools   52e670f670b0304f534b24f98c47ceb8a41bb710464414ebc9527ec71cc86aa4
    qtmultimedia        qtdeclarative   390f8e52ddee3aca5c4de7eead900c84c4fa61ff6d1f0ebea9c7543365c09b0a
    qtpositioning       qtdeclarative   d5e6b91801ae286e7630016caea3bdc5e1978b4291d6741d0d64c125650f78f5
    qtsvg               qtdeclarative   7f3cf02f4824bf03c2c5859ea6db173bf1482a1daf24e6cdf7bc78cfa26a8a94
    qtwayland           qtdeclarative   95788aa502f75441d4edf65932b235f76523084e13dbbb7b9ee2d207b32bd9b3
    qtlocation          qtpositioning   3791ce77299e6f600a593d0fbfa5bd32fbcfc2d16104782b84acc489e382e41b
    qtvirtualkeyboard   qtsvg           a1c6967b326243b2ca8d50bc7b7f7852c3975d9aa6ce4b186ebdf35bb1007e1c
)

while(QT_SRC)
    list(POP_FRONT QT_SRC QT_LIB_NAME DEPENDS_ON QT_LIB_HASH)

    ExternalProject_Add(${QT_LIB_NAME}
        URL https://download.qt.io/archive/qt/${QT_V}/${QT_VV}/submodules/${QT_LIB_NAME}-everywhere-src-${QT_VV}.tar.xz
        URL_HASH SHA256=${QT_LIB_HASH}
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND cmake . ${QT_SUBMODULE_CMAKE_ARGS}
        BUILD_COMMAND cmake --build . --parallel --config Release
        INSTALL_COMMAND cmake --install .
        DEPENDS ${DEPENDS_ON}
    )
endwhile()
