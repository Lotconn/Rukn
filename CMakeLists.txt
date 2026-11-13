cmake_minimum_required(VERSION 3.30...4.4)

project(Rukn
  VERSION 26.11
  DESCRIPTION "Your Simple and Beautiful Adhan App"
  LANGUAGES
    CXX
    C
)

# CheckIPOSupported for opportunistically enabling IPO/LTO
include(CheckIPOSupported)
include(FetchContent)

#
# Setup variables
#

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_PROJECT_SPDX_LICENSE "AGPL-3.0-or-later")

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

check_ipo_supported(RESULT IPOresult OUTPUT IPOoutput)
if (IPOresult)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
else ()
  message(STATUS "Not enabling IPO: ${IPOoutput}")
endif ()

# Ccache
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
  message(STATUS "Using Ccache")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif ()

set(CF_PROTECTION_FLAG -fcf-protection=none)
set(CF_PROTECTION_FLAG_MSVC "")

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
  message(STATUS "Enabling Intel CET for x86_64/amd64")
  set(CF_PROTECTION_FLAG -fcf-protection)
  set(CF_PROTECTION_FLAG_MSVC /guard:cf)
endif ()

if (LINUX)
  set(APP_LINUX_DIST "Default" CACHE STRING "Distribution Type")
  set(CACHE APP_LINUX_DIST PROPERTY STRINGS Default Flatpak)

  # optionally set from commandline or autodetects if omitted
  if (APP_LINUX_DIST STREQUAL Flatpak OR NOT "$ENV{FLATPAK_ID}" STREQUAL "")
    message(STATUS "Building for Flatpak: $ENV{FLATPAK_ID}")
    set(APP_FLATPAK_DIST ON CACHE BOOL "Flatpak Build")
  elseif (APP_LINUX_DIST STREQUAL Default)
    unset(APP_LINUX_DIST)
  endif ()
endif ()

configure_file(src/cpp/config.hpp.in config.hpp NO_SOURCE_PERMISSIONS @ONLY)

#
# Required libraries
#

# Qt
find_package(Qt6 REQUIRED
  COMPONENTS
    Core
    Gui
    LinguistTools
    Location
    Multimedia
    Positioning
    Qml
    Quick
    QuickControls2
    Widgets
  OPTIONAL_COMPONENTS QuickTools
)

if (LINUX)
  find_package(Qt6 REQUIRED COMPONENTS DBus WaylandClient)
endif ()

qt_standard_project_setup(REQUIRES 6.11 I18N_TRANSLATED_LANGUAGES ar bn)

set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)
set(QT_NO_MISSING_CATALOG_LANGUAGE_WARNING ON)

# Threads
find_package(Threads REQUIRED)

#
# External Libraries
#

# itay-grudev/SingleApplication
FetchContent_Declare(SingleApplication
  GIT_REPOSITORY https://github.com/itay-grudev/SingleApplication
  GIT_TAG v3.5.6
)
set(QT_DEFAULT_MAJOR_VERSION 6 CACHE STRING "Qt version to use, defaults to 6")
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")
FetchContent_MakeAvailable(SingleApplication)

# Lotconn/adhan-cpp
FetchContent_Declare(Adhan-cpp
  GIT_REPOSITORY https://github.com/lotconn/adhan-cpp
  GIT_TAG 545cac6f70a0a60f9d5f521dcbf2dda9eece3d79
)
FetchContent_MakeAvailable(Adhan-cpp)

# Lotconn/FileMan
FetchContent_Declare(FileMan
  GIT_REPOSITORY https://gitlab.com/lotconn/fileman
  GIT_TAG 830b671594be51131a3bc66b85454f7130174d8f
)
FetchContent_MakeAvailable(FileMan)

#
# Project Setup
#

add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/src/data/whatsnew.html
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND ${PROJECT_SOURCE_DIR}/gen_whatsnew
)

set(APP_QML_FILES
  src/qml/AdjustmentComponent.qml
  src/qml/AppModel.qml
  src/qml/AudioManComp.qml
  src/qml/CompassPage.qml
  src/qml/DateRow.qml
  src/qml/Line.qml
  src/qml/LocationInfoRow.qml
  src/qml/LocationSearchComponent.qml
  src/qml/Main.qml
  src/qml/MainPage.qml
  src/qml/PageButton.qml
  src/qml/PrayerRow.qml
  src/qml/SettingsPage.qml
  src/qml/SettingSubtitle.qml
  src/qml/SettingTitle.qml
  src/qml/SpinBoxComp.qml
  src/qml/TextButton.qml
  src/qml/Theme.qml
  src/qml/TimeLeftRow.qml
)

if (NOT TARGET Qt::QuickTools)
  list(APPEND APP_QML_FILES
    src/qml/icons/AsrIcon.qml
    src/qml/icons/DhuhrIcon.qml
    src/qml/icons/FajrIcon.qml
    src/qml/icons/IshaIcon.qml
    src/qml/icons/MaghribIcon.qml
  )
endif ()

set(APP_SOURCES
  src/cpp/alarm.h
  src/cpp/AppEngine.hpp src/cpp/AppEngine.cpp
  src/cpp/CoreEngine.hpp src/cpp/CoreEngine.cpp
  src/cpp/engine.h src/cpp/engine.cpp
  src/cpp/internal-helpers.hpp src/cpp/internal-helpers.cpp
  src/cpp/LocationEngine.hpp src/cpp/LocationEngine.cpp
  src/cpp/macos_window.h
  src/cpp/MultimediaEngine.hpp src/cpp/MultimediaEngine.cpp
  src/cpp/PrayerEngine.hpp src/cpp/PrayerEngine.cpp
  src/cpp/UiEngine.hpp src/cpp/UiEngine.cpp
  src/cpp/UpdateEngine.hpp src/cpp/UpdateEngine.cpp
)

if (APPLE)
  list(APPEND APP_SOURCES
    src/cpp/alarm_macos.cpp
    src/cpp/macos_window.mm
  )
elseif (LINUX)
  list(APPEND APP_SOURCES src/cpp/alarm_linux.cpp)
endif ()

set(APP_RESOURCES
  src/data/adhan-arRashid.ogg
  src/data/adhan-makkah.ogg
  src/data/fajr-adhan.ogg
  src/data/NotoNaskhArabic-VariableFont.ttf
  src/data/NotoSans-VariableFont.ttf
  src/data/NotoSansBengali-VariableFont.ttf
  src/data/whatsnew.html
  src/img/about.png
  src/img/arrow.png
  src/img/bg/0.jpg
  src/img/bg/1.jpg
  src/img/bg/2.jpg
  src/img/bg/3.jpg
  src/img/bg/4.jpg
  src/img/bg/5.jpg
  src/img/compass.png
  src/img/info.png
  src/img/minus.png
  src/img/plus.png
  src/img/prayerTimes/asr.svg
  src/img/prayerTimes/dhuhr.svg
  src/img/prayerTimes/fajr.svg
  src/img/prayerTimes/isha.svg
  src/img/prayerTimes/lastThirdNight.svg
  src/img/prayerTimes/maghrib.svg
  src/img/prayerTimes/midnight.svg
  src/img/prayerTimes/sunrise.svg
  src/img/rukn-symbolic.svg
  src/img/rukn.png
  src/img/settings.png
  src/img/start.png
  src/img/stop.png
)

foreach (filePath IN LISTS APP_QML_FILES APP_RESOURCES)
  string(REGEX REPLACE "^src/" "" qrc_filePath "${filePath}")
  set_source_files_properties("${filePath}" PROPERTIES QT_RESOURCE_ALIAS "${qrc_filePath}")
endforeach ()

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION src/cpp/main.cpp)

qt_add_qml_module(${PROJECT_NAME}
  URI App
  VERSION ${PROJECT_VERSION}
  QML_FILES "${APP_QML_FILES}"
  SOURCES "${APP_SOURCES}"
  RESOURCES "${APP_RESOURCES}"
)

qt_add_translations(${PROJECT_NAME}
  MERGE_QT_TRANSLATIONS
  SOURCE_TARGETS ${PROJECT_NAME}
  TS_FILE_BASE ${PROJECT_NAME_LOWER}
  TS_OUTPUT_DIRECTORY src/i18n/
  RESOURCE_PREFIX qt/qml/App/i18n
)

if (TARGET Qt::QuickTools)
  qt_target_qml_from_svg(${PROJECT_NAME}
    CURVE_RENDERER ASYNCHRONOUS_SHAPES OPTIMIZE_PATHS
    FILES
      src/img/prayerTimes/asr.svg
      src/img/prayerTimes/dhuhr.svg
      src/img/prayerTimes/fajr.svg
      src/img/prayerTimes/isha.svg
      src/img/prayerTimes/maghrib.svg
    OUTPUTS
      qml/AsrIcon.qml
      qml/DhuhrIcon.qml
      qml/FajrIcon.qml
      qml/IshaIcon.qml
      qml/MaghribIcon.qml
  )
endif ()

set_source_files_properties(src/qtquickcontrols2.conf PROPERTIES QT_RESOURCE_ALIAS qtquickcontrols2.conf
)
qt_target_qml_sources(${PROJECT_NAME} PREFIX "/" RESOURCES src/qtquickcontrols2.conf)

#
# Project Compilation
#

target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
  QT_ENABLE_STRICT_MODE_UP_TO=0x061200
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:
  -Wall
  -Wextra
  -Wp,-D_FORTIFY_SOURCE=3
  -Wp,-D_GLIBCXX_ASSERTIONS
  -fexceptions
  -fstack-protector-strong
  -fasynchronous-unwind-tables
  -fstack-clash-protection
  ${CF_PROTECTION_FLAG}
  -mno-omit-leaf-frame-pointer
  -pipe
  $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
  -grecord-gcc-switches
  -O1
  >>
  $<$<OR:$<CXX_COMPILER_ID:MSVC>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:
  /EHsc
  /GS
  /Oy-
  /W3
  ${CF_PROTECTION_FLAG_MSVC}
  >
)
target_include_directories(${PROJECT_NAME} PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/cpp>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  Lotconn::adhan
  FileManplugin
  Qt::Core
  Qt::Gui
  Qt::Location
  Qt::Multimedia
  Qt::Positioning
  Qt::Qml
  Qt::Quick
  Qt::QuickControls2
  Qt::Widgets
  SingleApplication::SingleApplication
  Threads::Threads
)

if (APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework AppKit")
endif ()

if (LINUX)
  target_link_libraries(${PROJECT_NAME} PRIVATE Qt::DBus Qt::WaylandClient)
endif ()

target_link_options(${PROJECT_NAME} PRIVATE
  $<$<AND:$<CXX_COMPILER_ID:Clang>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:-fuse-ld=lld>
  # ELF-specific linker hardening
  $<$<AND:$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:
  -Wl,-z,relro,-z,now
  -Wl,--as-needed
  -Wl,-Map=app.map
  >
  # MSVC / clang-cl linker hardening
  $<$<OR:$<CXX_COMPILER_ID:MSVC>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:
  ${CF_PROTECTION_FLAG_MSVC}
  >
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME_LOWER})
qt_finalize_target(${PROJECT_NAME})

#
# Packaging
#

if (LINUX)
  install(TARGETS ${PROJECT_NAME})
  install(DIRECTORY xdg/applications xdg/icons xdg/metainfo TYPE DATA)
endif ()
