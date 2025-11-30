cmake_minimum_required(VERSION 3.16)
project(kiosk-browser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets)

add_executable(kiosk-browser
    src/main.cpp
    src/BrowserWindow.cpp
    src/BrowserWindow.h
)

target_link_libraries(kiosk-browser
    Qt6::Widgets
    Qt6::WebEngineWidgets
)

# Install the kiosk-browser binary
install(TARGETS kiosk-browser DESTINATION bin)
