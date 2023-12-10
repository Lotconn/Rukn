cmake_minimum_required(VERSION 3.27)

project(munadi LANGUAGES CXX C)
set(APP_VERSION 22.04)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(src/cpp/version.h.in version.h)

find_package(Qt6
    COMPONENTS
        Core Gui Widgets Quick QuickControls2 Multimedia Location Positioning REQUIRED)

find_package(Threads)

qt_standard_project_setup()


if(LINUX)
    find_package(Qt6 COMPONENTS DBus REQUIRED)

    set(KDSingleApplication_QT6 ON)
    set(KDSingleApplication_STATIC ON)
    add_subdirectory(libs/KDSingleApplication)
endif()


set(TS_FILES
    src/i18n/munadi_ar.ts)

set(HEADERS
    src/cpp/alarms.h
    src/cpp/engine.h
    src/cpp/updater.h
    src/cpp/libitl/hijri.h)

set(SOURCES
    src/cpp/alarms.cpp
    src/cpp/engine.cpp
    src/cpp/updater.cpp
    src/cpp/main.cpp
    src/cpp/libitl/hijri.c)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    qt6_add_resources(QRC_FILES src/qml.qrc src/rc.qrc)
else()
    #qtquick_compiler_add_resources(QRC_FILES src/qml.qrc src/rc.qrc)
    qt6_add_resources(QRC_FILES src/qml.qrc src/rc.qrc)
endif()


if(ANDROID)
    qt_add_executable(${PROJECT_NAME}
        src/android/AndroidManifest.xml
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

if(ANDROID)
    include(${CMAKE_CURRENT_SOURCE_DIR}/libs/android/openssl/android_openssl.cmake)
    #set(QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/android")

    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
    )

    add_android_openssl_libraries(${PROJECT_NAME})
endif()



target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
    PUBLIC Qt6::Core Qt6::Widgets Qt6::Quick Qt6::QuickControls2 Qt6::Multimedia Qt6::Location
    Qt6::Positioning Threads::Threads )

if(LINUX)
    target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::DBus kdsingleapplication)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

install(TARGETS ${PROJECT_NAME})
install(DIRECTORY xdg/icons xdg/metainfo xdg/applications TYPE DATA)
