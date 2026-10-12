cmake_minimum_required(VERSION 3.16...4.4)

project(munadi
  VERSION 26.11
  DESCRIPTION "Your Simple and Beautiful Athan App"
  LANGUAGES CXX C
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

string(TOLOWER ${PROJECT_NAME} PROEJCT_NAME_LOWER)

check_ipo_supported(RESULT IPOresult OUTPUT IPOoutput)
if(IPOresult)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
else()
  message(STATUS "Not enabling IPO: ${IPOoutput}")
endif()

# Ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(CF_PROTECTION_FLAG -fcf-protection=none)
set(CF_PROTECTION_FLAG_MSVC "")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
  message(STATUS "Enabling Intel CET for x86_64/amd64")
  set(CF_PROTECTION_FLAG -fcf-protection)
  set(CF_PROTECTION_FLAG_MSVC /guard:cf)
endif()

configure_file(src/cpp/version.hpp.in version.hpp NO_SOURCE_PERMISSIONS @ONLY)

#
# Required libraries
#

# Qt
find_package(Qt6
  REQUIRED
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
)

if(LINUX)
  find_package(Qt6 REQUIRED COMPONENTS DBus WaylandClient)
endif()

qt_standard_project_setup(REQUIRES 6.12 I18N_TRANSLATED_LANGUAGES ar bn)

set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

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
set(QAPPLICATION_CLASS FreeStandingSingleApplication CACHE STRING
  "Inheritance class for SingleApplication"
)
FetchContent_MakeAvailable(SingleApplication)

# Lotconn/adhan-cpp
FetchContent_Declare(Adhan-cpp
  GIT_REPOSITORY https://github.com/lotconn/adhan-cpp
  GIT_TAG bf13ff4cbcc855756111cad1ee3d392d83c8b153
)
FetchContent_MakeAvailable(Adhan-cpp)

# Lotconn/FileMan
FetchContent_Declare(FileMan
  GIT_REPOSITORY https://gitlab.com/lotconn/fileman
  GIT_TAG 830b671594be51131a3bc66b85454f7130174d8f
)
# set(FM_BUILD_AS "STATIC" CACHE STRING "Build FileMan as static")
FetchContent_MakeAvailable(FileMan)

if(ANDROID)
  FetchContent_Declare(android_openssl
    DOWNLOAD_EXTRACT_TIMESTAMP true
    URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
  )
  FetchContent_MakeAvailable(android_openssl)
  include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)

  set_property(TARGET
    ${PROJECT_NAME}
    APPEND
    PROPERTY
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
  )
  add_android_openssl_libraries(${PROJECT_NAME})
endif()

#
# Project Setup
#

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/data/whatsnew.html
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND ${PROJECT_SOURCE_DIR}/gen_whatsnew
)

set(MUNADI_QML_FILES
  src/qml/AdjustmentComponent.qml
  src/qml/AudioManComp.qml
  src/qml/CompassPage.qml
  src/qml/DateRow.qml
  src/qml/Line.qml
  src/qml/LocationInfoRow.qml
  src/qml/LocationSearchComponent.qml
  src/qml/LocationService.qml
  src/qml/Main.qml
  src/qml/MainPage.qml
  src/qml/MunadiModel.qml
  src/qml/PageButton.qml
  src/qml/PrayerRow.qml
  src/qml/SettingsPage.qml
  src/qml/SettingSubtitle.qml
  src/qml/SettingTitle.qml
  src/qml/TextButton.qml
  src/qml/Theme.qml
  src/qml/TimeLeftRow.qml
)

set(MUNADI_JS_FILES
  src/data/Adhan.js
)

set(MUNADI_SOURCES
  src/cpp/alarm.h
  src/cpp/engine.h
  src/cpp/CoreEngine.hpp
  src/cpp/PrayerEngine.hpp
  src/cpp/MultimediaEngine.hpp
  src/cpp/UiEngine.hpp
  src/cpp/internal-helpers.hpp
  src/cpp/macos_window.h
  src/cpp/updater.h
  src/cpp/engine.cpp
  src/cpp/internal-helpers.cpp
  src/cpp/CoreEngine.cpp
  src/cpp/PrayerEngine.cpp
  src/cpp/MultimediaEngine.cpp
  src/cpp/UiEngine.cpp
  src/cpp/updater.cpp
)

if(APPLE)
  list(APPEND
    MUNADI_SOURCES
    src/cpp/alarm_macos.cpp
    src/cpp/macos_window.mm
  )
elseif(LINUX)
  list(APPEND MUNADI_SOURCES src/cpp/alarm_linux.cpp)
endif()

set(MUNADI_RESOURCES
  src/data/athan-arRashid.ogg
  src/data/athan-makkah.ogg
  src/data/fajr-athan.ogg
  src/data/NotoNaskhArabic-VariableFont.ttf
  src/data/NotoSans-VariableFont.ttf
  src/data/NotoSansBengali-VariableFont.ttf
  src/data/timezones.tsv
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
  src/img/munadi-symbolic.svg
  src/img/munadi.png
  src/img/plus.png
  src/img/settings.png
  src/img/start.png
  src/img/stop.png
)

foreach(filePath IN LISTS MUNADI_JS_FILES MUNADI_QML_FILES MUNADI_RESOURCES)
  string(REGEX REPLACE "^src/" "" qrc_filePath "${filePath}")
  set_source_files_properties("${filePath}"
    PROPERTIES
    QT_RESOURCE_ALIAS "${qrc_filePath}"
  )
endforeach()

qt_add_executable(${PROJECT_NAME} src/cpp/main.cpp)
qt_add_qml_module(${PROJECT_NAME}
  URI Munadi
  VERSION ${PROJECT_VERSION}
  QML_FILES
  "${MUNADI_JS_FILES}"
  "${MUNADI_QML_FILES}"
  SOURCES
  "${MUNADI_SOURCES}"
  RESOURCES
  "${MUNADI_RESOURCES}"
)

qt_add_translations(${PROJECT_NAME}
  SOURCE_TARGETS ${PROJECT_NAME}
  TS_FILE_BASE ${PROEJCT_NAME_LOWER}
  TS_FILE_DIR src/i18n/
  RESOURCE_PREFIX qt/qml/Munadi/i18n
)

set_source_files_properties(src/qtquickcontrols2.conf
  PROPERTIES
  QT_RESOURCE_ALIAS qtquickcontrols2.conf
)
qt_target_qml_sources(${PROJECT_NAME}
  PREFIX "/"
  RESOURCES
  src/qtquickcontrols2.conf
)

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
  adhan::adhan
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

if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework AppKit")
endif()

if(LINUX)
  target_link_libraries(${PROJECT_NAME} PRIVATE Qt::DBus Qt::WaylandClient)
endif()

target_link_options(${PROJECT_NAME} PRIVATE
  $<$<AND:$<CXX_COMPILER_ID:Clang>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:-fuse-ld=lld>
  # ELF-specific linker hardening
  $<$<AND:$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:
  -Wl,-z,relro,-z,now
  -Wl,--as-needed
  -Wl,-Map=munadi.map
  >
  # MSVC / clang-cl linker hardening
  $<$<OR:$<CXX_COMPILER_ID:MSVC>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:
  ${CF_PROTECTION_FLAG_MSVC}
  >
)

#
# Packaging
#

if(LINUX)
  install(TARGETS ${PROJECT_NAME})
  install(DIRECTORY xdg/icons xdg/metainfo xdg/applications TYPE DATA)
endif()
