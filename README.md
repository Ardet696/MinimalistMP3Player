# Minimalist MP3 Player

#### Designed for great performance and low memory/cpu usage with C++20.

![Default TUI](images/MP3Player.png)

---

## Install

### Pre-built binaries (easiest)

Pre-built binaries for Linux (x86_64) and macOS (arm64) are available on the [GitHub Releases](https://github.com/ardet696/MinimalistMP3Player/releases) page. Download the binary for your platform, make it executable, and run. You only need SDL2 installed:

| Platform | Install SDL2             |
|----------|--------------------------|
| Arch | sudo pacman -S sdl2      |
| Ubuntu / Debian | sudo apt install libsdl2-2.0-0 |
| Fedora | sudo dnf install SDL2    |
| macOS | brew install sdl2        |

```bash
chmod +x mp3player-linux-x86_64
./mp3player-linux-x86_64
```

### Arch Linux (AUR)

```bash
yay -S minimalist-mp3-player
```

### Build from source

Install dependencies:

| Distro | Command |
|--------|---------|
| Arch | sudo pacman -S sdl2 cmake gcc |
| Ubuntu / Debian | sudo apt install libsdl2-dev cmake g++|
| Fedora | sudo dnf install SDL2-devel cmake gcc-c++|

```bash
git clone https://github.com/ardet696/MinimalistMP3Player.git
cd MinimalistMP3Player

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

./build/MP3Player
```

> Note: Avoid using sudo cmake --install build if you plan to use the AUR package later — both install to different paths and the manual install takes priority.

### macOS

Apple Clang supports `std::jthread` (with `-std=c++20`) as of Apple Clang 21, so you can build natively with the system toolchain — no extra compiler required. Check your version:

```bash
clang++ --version
```

If it reports Apple Clang 21 or newer (shipped with macOS 26.2+), just install CMake and build — SDL2 is downloaded and built automatically:

```bash
brew install cmake

./scripts/build-ftxui.sh

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)

./build/MP3Player
```

If your Apple Clang is older than 21, update the Command Line Tools (System Settings → General → Software Update, or `xcode-select --install` / `softwareupdate --list`) and re-check `clang++ --version`.

If you cannot upgrade, fall back to GCC via Homebrew:

```bash
brew install gcc cmake

./scripts/build-ftxui.sh

ls /opt/homebrew/bin/g++-*

cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-14
cmake --build build -j$(sysctl -n hw.ncpu)

./build/MP3Player
```

---

### First launch? You need to set your music root directory.

The player is built around the concept of an "Album Player" or "Playlist Player". It looks for directories containing MP3 files inside your music root each subdirectory is treated as an album or playlist.

- On first launch the file manager will be empty.
- Type `RootConfig` in the command bar and press Enter, then type the full path to your music directory (e.g. `/home/user/Music`) and press Enter again.

In the screenshot below, the left side shows detected albums; the right side shows the contents of that music directory. Non-MP3 files (images, etc.) are ignored.

![RootExample](images/RootAlbums.png)

### Command terminal emulator

The command panel works like a mini config terminal.

Some commands: `play`, `stop`, `next`, `prev`, `help`, `fileHelp`.

Commands like `volume`, `output`, `visuals`, `themes`, and `RootConfig` enter an interactive mode that waits for your selection. Press `Esc` at any time to cancel and return to normal command input.
