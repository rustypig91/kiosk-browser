cmake_minimum_required(VERSION 3.16)
project(kiosk-browser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    set(CMAKE_VERBOSE_MAKEFILE ON)
else()
    add_compile_definitions(NDEBUG QT_NO_DEBUG_OUTPUT)
endif()

option(USE_QT6 "Build with Qt6" OFF)

if(USE_QT6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets)
    set(QT_LIBRARIES Qt6::Widgets Qt6::WebEngineWidgets)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets WebKit WebSockets)
    set(QT_LIBRARIES Qt5::Widgets Qt5::WebKit Qt5WebKitWidgets Qt5::WebSockets)
endif()


if (USE_QT6)
    add_executable(kiosk-browser
        src/main.cpp
        src/Qt6BrowserWindow.cpp
        src/Qt6BrowserWindow.h
    )
else()
    add_executable(kiosk-browser
        src/main.cpp
        src/Qt5BrowserWindow.cpp
        src/Qt5BrowserWindow.h
    )
endif()

target_link_libraries(kiosk-browser PRIVATE ${QT_LIBRARIES})

# Install the kiosk-browser binary
install(TARGETS kiosk-browser DESTINATION bin)
