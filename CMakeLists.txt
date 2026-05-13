cmake_minimum_required(VERSION 3.30)

project(munadi LANGUAGES CXX C)
set(APP_VERSION 26.04)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(LINUX)
    set(CMAKE_CXX_FLAGS "-march=native -fomit-frame-pointer -fstack-clash-protection")
endif()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

configure_file(src/cpp/version.h.in version.h)

find_package(Qt6
    COMPONENTS
        Core Gui Widgets Quick QuickControls2 Multimedia Location Positioning )

find_package(Threads)

qt_standard_project_setup()

if(LINUX)
    find_package(Qt6 COMPONENTS DBus REQUIRED)

    set(KDSingleApplication_EXAMPLES OFF CACHE BOOL "Build the examples")
    set(KDSingleApplication_STATIC ON CACHE BOOL "Build lib as static")
    add_subdirectory(libs/KDSingleApplication)
endif()

set(TS_FILES
    src/i18n/munadi_ar.ts)

set(HEADERS
    src/cpp/alarms.h
    src/cpp/dummysapp.h
    src/cpp/engine.h
    src/cpp/macos_window.h
    src/cpp/updater.h)

set(SOURCES
    src/cpp/engine.cpp
    src/cpp/updater.cpp
    src/cpp/main.cpp)

if(APPLE)
    list(APPEND SOURCES src/cpp/alarms_macos.cpp src/cpp/macos_window.mm)
else()
    list(APPEND SOURCES src/cpp/alarms_linux.cpp)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    qt6_add_resources(QRC_FILES src/qml.qrc src/rc.qrc)
else()
    qt6_add_resources(QRC_FILES src/qml.qrc src/rc.qrc)
endif()

if(ANDROID)
    qt_add_executable(${PROJECT_NAME}
        ${HEADERS}
        ${SOURCES}
        ${TS_FILES}
        ${QRC_FILES})
else()
    qt_add_executable(${PROJECT_NAME}
        ${HEADERS}
        ${SOURCES}
        ${TS_FILES}
        ${QRC_FILES})
endif()

if (ANDROID)
    include(FetchContent)
    FetchContent_Declare(
        android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(android_openssl)
    include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)

    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
    add_android_openssl_libraries(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
    PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Quick Qt6::QuickControls2
    Qt6::Multimedia Qt6::Location Qt6::Positioning Threads::Threads)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC "-framework AppKit")
endif()

if(LINUX)
    find_package(Qt6 COMPONENTS WaylandClient)

    target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::WaylandClient Qt6::DBus kdsingleapplication)
    #target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

    install(TARGETS ${PROJECT_NAME})
    install(DIRECTORY xdg/icons xdg/metainfo xdg/applications TYPE DATA)
endif()
