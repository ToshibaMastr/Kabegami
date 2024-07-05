# Kabegami ❖

![demo](./docs/demo.gif)

**Kabegami** is an application for setting video wallpapers on your desktop.

## Installation
### 1. Install Dependencies
```sh
sudo apt-get update
sudo apt-get install build-essential cmake libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libx11-dev libxrandr-dev
```

### 2. Build
```sh
git clone https://github.com/ToshibaMastr/Kabegami
cd Kabegami
mkdir build && cd build
cmake .. && make
```

### 3. Install
```sh
sudo -E make install
```

### 4. Uninstall
```sh
sudo make uninstall
```

## Quick Start

Run **Kabegami** with `video.mp4` wallpaper and infinite loop:

```sh
kabegami --loop video.mp4
```

## Documentation

For information about available options, use:

```sh
kabegami --help
```

## License
This project is licensed under the terms of the GNU General Public License v3.0. For details, see the [LICENSE](LICENSE) file.
