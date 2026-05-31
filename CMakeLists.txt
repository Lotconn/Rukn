cmake_minimum_required(VERSION 3.16...4.3)

project(munadi
  VERSION     26.04
  DESCRIPTION "A Simple cross-platoform athan app"
  LANGUAGES   CXX C
)

set(CMAKE_CXX_STANDARD  17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION  ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG  OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO OFF)
set(CMAKE_PROJECT_SPDX_LICENSE "AGPL-3.0-or-later")

configure_file(src/cpp/version.hpp.in version.hpp NO_SOURCE_PERMISSIONS @ONLY)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

find_package(Qt6
  REQUIRED
  COMPONENTS
    Core
    Gui
    Location
    Multimedia
    Positioning
    Qml
    Quick
    QuickControls2
    Widgets
)
find_package(Threads REQUIRED)

qt_standard_project_setup(REQUIRES 6.9)

if(LINUX)
  find_package(Qt6 REQUIRED COMPONENTS DBus)

  option(KDSingleApplication_STATIC "Build lib as static" ON)
  add_subdirectory(libs/KDSingleApplication)
endif()

set(FM_BUILD_AS "STATIC" CACHE STRING "Build FileMan as static")
add_subdirectory(libs/HasanEsa/FileMan)

set(MUNADI_QML_FILES
  src/qml/AdjustmentComponent.qml
  src/qml/AudioManComp.qml
  src/qml/CompassPage.qml
  src/qml/DateRow.qml
  src/qml/Line.qml
  src/qml/LocationInfoRow.qml
  src/qml/LocationSearchComponent.qml
  src/qml/LocationService.qml
  src/qml/main.qml
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
  src/cpp/dummysapp.h
  src/cpp/engine.cpp
  src/cpp/engine.h
  src/cpp/macos_window.h
  src/cpp/updater.cpp
  src/cpp/updater.h
)

if(APPLE)
  list(APPEND
    MUNADI_SOURCES
      src/cpp/alarm_macos.cpp
      src/cpp/macos_window.mm
  )
else()
  list(APPEND MUNADI_SOURCES src/cpp/alarm_linux.cpp)
endif()

set(MUNADI_RESOURCES
  src/data/athan-arRashid.ogg
  src/data/athan-makkah.ogg
  src/data/fajr-athan.ogg
  src/data/NotoNaskhArabicUI-Bold.ttf
  src/data/NotoNaskhArabicUI-Regular.ttf
  src/data/NotoSans-Bold.ttf
  src/data/NotoSans-ExtraLight.ttf
  src/data/NotoSans-Regular.ttf
  src/data/timezones.tsv
  src/data/whatsnew.html
  src/i18n/munadi_ar.qm
  src/i18n/munadi_ar.ts
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
  URI                   Munadi
  VERSION               ${PROJECT_VERSION}
  QML_FILES
    "${MUNADI_JS_FILES}"
    "${MUNADI_QML_FILES}"
  SOURCES
    "${MUNADI_SOURCES}"
  RESOURCES
    "${MUNADI_RESOURCES}"
  DEPENDENCIES  TARGET  FileMan
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

if (ANDROID)
  set_property(TARGET
      ${PROJECT_NAME}
    APPEND
      PROPERTY
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
  )
  add_subdirectory(libs/android/openssl)
  add_android_openssl_libraries(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
    QT_ENABLE_STRICT_MODE_UP_TO=0x060900
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<PLATFORM_ID:Linux>>:
    -Wp,-D_FORTIFY_SOURCE=3
    -Wp,-D_GLIBCXX_ASSERTIONS
    -fexceptions
    -fstack-protector-strong
    -fasynchronous-unwind-tables
    -fstack-clash-protection
    -fcf-protection
    -mno-omit-leaf-frame-pointer
    -pipe
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
      -grecord-gcc-switches
    >
  >
)
target_include_directories(${PROJECT_NAME} PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/cpp>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
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
  Threads::Threads
)
target_link_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-fuse-ld=lld>
  $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:
    -Wl,-z,relro,-z,now
    -Wl,--as-needed
  >
)

if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework AppKit")
endif()

if(LINUX)
  find_package(Qt6 COMPONENTS WaylandClient)

  target_link_libraries(${PROJECT_NAME} PRIVATE
    kdsingleapplication
    Qt::DBus
    Qt::WaylandClient
  )

  install(TARGETS ${PROJECT_NAME})
  install(DIRECTORY xdg/icons xdg/metainfo xdg/applications TYPE DATA)
endif()
