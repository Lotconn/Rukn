cmake_minimum_required(VERSION 3.14)

project(munadi LANGUAGES CXX C)
set(APP_VERSION 21.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(ANDROID)
    include(${CMAKE_CURRENT_SOURCE_DIR}/libs/android/openssl/CMakeLists.txt)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/android")
endif()

configure_file(src/cpp/version.h.in version.h)

find_package(Qt5
    COMPONENTS
        Core Gui Widgets Quick QuickControls2 Multimedia Location Positioning DBus REQUIRED)

find_package(Qt5QuickCompiler)
find_package(Threads)

set(KDSingleApplication_STATIC ON)
add_subdirectory(libs/KDSingleApplication)


set(TS_FILES
    src/i18n/munadi_ar.ts)

set(HEADERS
    src/cpp/alarms.h
    src/cpp/engine.h
    src/cpp/updater.h)

set(SOURCES
    src/cpp/alarms.cpp
    src/cpp/engine.cpp
    src/cpp/updater.cpp
    src/cpp/main.cpp
    src/cpp/libitl/hijri.c)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    qt5_add_resources(QRC_FILES src/qml.qrc src/rc.qrc)
else()
    qtquick_compiler_add_resources(QRC_FILES src/qml.qrc src/rc.qrc)
endif()


if(ANDROID)
    add_library(${PROJECT_NAME} SHARED
        src/android/AndroidManifest.xml
        ${HEADERS}
        ${SOURCES}
        ${TS_FILES}
        ${QRC_FILES})
else()
    add_executable(${PROJECT_NAME}
        ${HEADERS}
        ${SOURCES}
        ${TS_FILES}
        ${QRC_FILES})
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt5::Core Qt5::Widgets Qt5::Quick Qt5::QuickControls2 Qt5::Multimedia Qt5::Location
    Qt5::Positioning Qt5::DBus kdsingleapplication Threads::Threads )

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

install(TARGETS ${PROJECT_NAME})

install(
    DIRECTORY
        xdg/icons
        xdg/metainfo
        xdg/applications
    TYPE DATA)
