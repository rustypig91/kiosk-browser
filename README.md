# kiosk-browser

A minimal kiosk-mode web browser built with Qt6 in C++.

Requires Qt6 libraries and CMake.

## Directory layout

- `src/` – source code
- `CMakeLists.txt` – build script

#### Setup example on Ubuntu

Install the required Qt6 packages:

```bash
sudo apt update
sudo apt install qt6-base-dev qt6-webengine-dev cmake build-essential
```

### Building the project

Run these commands from the project root:

```bash
mkdir build
cd build
cmake ..
make
./kiosk-browser  # Or .\kiosk-browser.exe on Windows
```
