cmake_minimum_required(VERSION 3.30...4.4)

project(external_libs)

include(ExternalProject)
include(ProcessorCount)

ProcessorCount(NPROC)

set(QT_V 6.12)
set(QT_VV 6.12.0)
set(FFMPEG_V 9.0.1) # Qt 6.12 recommends FFmpeg 9.0.1
set(INSTALL_PREFIX /usr)

set(FFMPEG_CONF
    ./configure

    # Standard options:
    --fatal-warnings # blocks undesired silent passes
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
    --disable-cuda-llvm # acceleration isn't necessary in ffmpeg at this point
    --disable-iconv
    --disable-libxcb #
    --disable-xlib # disable all x11 dependencies

    # Toolchain options: using clang+llvm; other tools need to be explicitly defined
    --nm=llvm-nm
    --ar=llvm-ar
    --cc=clang
    --cxx=clang++
    --ld=clang
    --ranlib=llvm-ranlib
    --enable-lto

    # Advanced options:

    # don't use single quotes only
    # extra hardening flags were added
    --extra-cflags="' -O3 -pipe -Wp,-D_FORTIFY_SOURCE=3 -fasynchronous-unwind-tables -fstack-protector-strong -fcf-protection -fstack-clash-protection -mno-omit-leaf-frame-pointer '"
    --extra-ldflags="' -fuse-ld=lld -Wl,-z,relro,-z,now -Wl,--as-needed '"

    # Developer options:
    --disable-debug
)

# FFmpeg
ExternalProject_Add(ffmpeg
    URL https://www.ffmpeg.org/releases/ffmpeg-${FFMPEG_V}.tar.xz
    URL_HASH SHA256=cf38e0e28c7e5605942c4a77755349b0145804a397af37eb1fb4c77cb237f635
    BUILD_IN_SOURCE ON
    CONFIGURE_COMMAND ${FFMPEG_CONF}
    BUILD_COMMAND make -j${NPROC}
    INSTALL_COMMAND make install
    STEP_TARGETS download
    DOWNLOAD_NO_PROGRESS ON
)

set(QT_CONF
    ./configure

    # Top-level installation directories:
    -prefix ${INSTALL_PREFIX}

    # Configure meta:
    # these tools are not needed in linux-only builds
    -no-feature-androiddeployqt
    -no-feature-harmonydeployqt
    -no-feature-qmake
    -no-feature-printsupport
    -no-feature-wasmdeployqt

    # Build options:
    -cmake-generator Ninja
    -release
    -static
    -no-sbom
    -reduce-exports
    -reduce-relocations
    -ltcg
    -linker lld # using clang+llvm with lld
    -ccache

    # Component selection:
    -nomake examples
    -nomake tests
    -nomake benchmarks
    -nomake manual-tests
    -nomake minimal-static-tests
    -nomake doc-snippets
    -force-bundled-libs # _always_ use the bundled libs

    # Core options:
    -qt-doubleconversion
    -no-glib
    -no-inotify
    -no-icu
    -no-jemalloc
    -qt-pcre
    -qt-zlib

    # Network options:
    -ssl # use system-installed OpenSSL (shared) lib

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

    # Build environment
    CMAKE_C_FLAGS=-pipe #
    CMAKE_CXX_FLAGS=-pipe # quotes are necessary
    CMAKE_C_COMPILER=clang
    CMAKE_CXX_COMPILER=clang++
    FEATURE_optimize_full=ON
    FEATURE_optimized_tools=ON
    FEATURE_egl_x11=OFF #
    FEATURE_xlib=OFF # these two help to disable X11 deps completely
    QT_BUILD_DOCS=OFF
    QT_BUILD_TOOLS_BY_DEFAULT=OFF
)

# Qt
ExternalProject_Add(qtbase
    URL https://download.qt.io/archive/qt/${QT_V}/${QT_VV}/submodules/qtbase-everywhere-src-${QT_VV}.tar.xz
    URL_HASH SHA256=a951bd163c7b80fc6b8c88d7668fb56abf91c152373e13c10666763238131307
    BUILD_IN_SOURCE ON
    CONFIGURE_COMMAND ${QT_CONF} && cat config.summary
    BUILD_COMMAND cmake --build . --parallel
    INSTALL_COMMAND cmake --install .
    STEP_TARGETS download
    # suppress unnecessary noise
    DOWNLOAD_NO_PROGRESS ON
)

# Modules may have multiple dependencies; see dependencies.yaml in each module's archive
set(QT_SRC
    # Module            Depnds on       SHA256
    qtimageformats      qtbase          0943132b5db8de6b6a7f1d162682173e8763c7ed94e80fd39113292845f67762
    qtlanguageserver    qtbase          552119e37360c06c5fb0b54e91bbfdf8da5a80d2cb1441ef93e966eb3b7a426d
    qtshadertools       qtbase          c7d84f436e1aaef39fdcadebf2bd71bdf24dc69497e13f2e0198873cbbfea2ab
    qtsvg               qtbase          e4ab39534ec97987b1b9b60ef7f4d3253d5912a69f8c713a01753174a4029331
    qtdeclarative       qtbase,qtimageformats,qtlanguageserver,qtshadertools,qtsvg 311f3a2603e1973bb59baef9dfa740a376de713157d4782ee043681e889c9260
    qtmultimedia        ffmpeg,qtdeclarative 3143f53b64257ba2a0685c940f5dce476c75871ff86b516de45cfd54ea93a62d
    qtpositioning       qtdeclarative   b060d410fac7413b05a0e7938bbf3325f7ad08d7df14af3956270f0696e76763
    qttools             qtdeclarative   8dab8f3611496486a470ad5f115ceea584f36bc22a2b8b6f6ebdbafbb8160693
    qtwayland           qtdeclarative   ae14d9bcf1bb3c300a3ab2e6a53f50284e5c6c966a4ac4cc5aedcb1bc74e4d02
    qtlocation          qtpositioning   218696d57c9eb95e8756e9fda62a8eabf94badc507db5f6e8ab9a640c4b0f50d
)

# Flags passed to every sub-module cmake invocation, unique or common to each module.

# As cmake args:
set(QT_CMAKE_ARGS -DQT_BUILD_DOCS=OFF -DQT_GENERATE_SBOM=OFF)
set(QT_CMAKE_ARGS-qtdeclarative -DFEATURE_qml_debug=OFF)
set(QT_CMAKE_ARGS-qtmultimedia -DFFMPEG_DIR=/usr -DFEATURE_linux_v4l=OFF -DFEATURE_pipewire=ON)

# As options in `configure` helper script
set(QT_CONFIG_ARGS-qttools
    -no-feature-assistant
    -no-feature-designer
    -no-feature-distancefieldgenerator
    -no-feature-fullqthelp
    -no-feature-kmap2qmap
    -no-feature-pixeltool
    -no-feature-qdoc
)
set(QT_CONFIG_ARGS-qtwayland -no-feature-wayland-compositor-quick)

while(QT_SRC)
    list(POP_FRONT QT_SRC QT_LIB_NAME DEPENDS_ON QT_LIB_HASH)
    string(REPLACE "," ";" DEPENDS_ON "${DEPENDS_ON}")

    ExternalProject_Add(${QT_LIB_NAME}
        URL https://download.qt.io/archive/qt/${QT_V}/${QT_VV}/submodules/${QT_LIB_NAME}-everywhere-src-${QT_VV}.tar.xz
        URL_HASH SHA256=${QT_LIB_HASH}
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND /usr/bin/qt-configure-module . ${QT_CONFIG_ARGS-${QT_LIB_NAME}} -- ${QT_CMAKE_ARGS} ${QT_CMAKE_ARGS-${QT_LIB_NAME}} && cat config.summary && cmake -N -L .
        BUILD_COMMAND cmake --build . --parallel
        INSTALL_COMMAND cmake --install .
        STEP_TARGETS download
        DOWNLOAD_NO_PROGRESS ON
    )
    ExternalProject_Add_StepDependencies(${QT_LIB_NAME} configure ${DEPENDS_ON})
    list(APPEND FETCH_TARGETS ${QT_LIB_NAME}-download)
endwhile()

# Parallelize downloads
add_custom_target(fetch-all DEPENDS ffmpeg-download qtbase-download ${FETCH_TARGETS})
