# Minimalist MP3 Player

#### Simple and minimalist MP3 player, designed for great performance and low memory/cpu usage with C++20.

![Default TUI](images/DefaultTUI.png)

---

## Install

### Pre-built binaries (easiest)

Pre-built binaries for Linux (x86_64) and macOS (arm64) are available on the [GitHub Releases](https://github.com/ardet696/MinimalistMP3Player/releases) page. Download the binary for your platform, make it executable, and run. You only need SDL2 installed:

| Platform | Install SDL2 |
|----------|-------------|
| Arch | `sudo pacman -S sdl2` |
| Ubuntu / Debian | `sudo apt install libsdl2-2.0-0` |
| Fedora | `sudo dnf install SDL2` |
| macOS | `brew install sdl2` |

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
| Arch | `sudo pacman -S sdl2 cmake gcc` |
| Ubuntu / Debian | `sudo apt install libsdl2-dev cmake g++` |
| Fedora | `sudo dnf install SDL2-devel cmake gcc-c++` |

```bash
# 1. Clone the repository (includes the FTXUI dependency)
git clone --recursive https://github.com/ardet696/MinimalistMP3Player.git
cd MinimalistMP3Player

# 2. Build the release version
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 3. (Optional) Install system-wide so you can launch it from any terminal
sudo cmake --install build
```

After step 2, run the player with `./build/MP3Player`.
After step 3, just type `MP3Player` from anywhere.

### macOS

Apple Clang does not support `std::jthread`. macOS users must install GCC via Homebrew:

```bash
# 1. Install dependencies
brew install gcc sdl2 cmake

# 2. Find your GCC version
ls /opt/homebrew/bin/g++-*    # e.g. g++-14, g++-16

# 3. Build with GCC instead of Apple Clang (replace 14 with your version)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-14
cmake --build build -j$(sysctl -n hw.ncpu)

# 4. Run
./build/MP3Player
```

### Windows

The codebase is cross-platform (SDL2, FTXUI, and minimp3 all support Windows), but has only been tested on Linux. Contributions and testing for Windows are welcome.

---

## How to use

### First launch — set your music root

On first launch the file manager will be empty. Type `RootConfig` in the command bar and press `Enter`, then type the full path to your music directory (e.g. `/home/user/Music`) and press `Enter` again. The path is persisted across sessions. Only directories that contain `.mp3` files directly (no subdirectories) are recognised as albums.

### Album Manager (left panel)

| Key | Action |
|-----|--------|
| `↑` / `↓` or scroll | Navigate albums / songs |
| `Enter` | Open album and start playing from track 1 |
| `Enter` (on a song) | Play selected song |
| `Backspace` | Go back to album list |

The currently playing track is highlighted in the song list with a `▶ Now playing:` indicator.

### Playback commands (command bar, bottom)

Type a command and press `Enter`:

| Command | Action |
|---------|--------|
| `play` | Resume playback |
| `stop` | Pause playback |
| `next` | Skip to next track |
| `prev` | Go back to previous track |
| `output` | Select audio output device |
| `RootConfig` | Change the music root directory |
| `themes` | Enter theme selection mode |
| `help` | Show available commands |
| `quit` / `q` | Exit the player |

### Playing Bar

The playing bar shows a real-time spectrum visualiser and a progress bar. Bar decay speed adapts to the BPM of the current track.

### Themes
The changes of theme of the overall terminal emulator are due to my linux distribution (Omarchy). But within the TUI app you can change the colors of the different features like the playing bar or audio spectrum.
Type `themes` and press `Enter`, then enter a number `1`–`4` to switch the colour palette:

| # | Theme |
|---|-------|
| 1 | Fire |
| 2 | BW |
| 3 | PurpleRain |
| 4 | Forest |

The palette applies to whatever terminal colour scheme you are running. Below are examples taken from different [Omarchy](https://omarchy.org) themes:

The "visuals" panel within the TUI is currently under development. Feel free to add a recommendation of what could be added to that visual section.
![Black theme](images/WithMyOmarchyTheme+ColorBlack.png)
![Green theme](images/WithMyOmarchyTheme+ColorGreen.png)
![Purple theme](images/WithMyOmarchyTheme+ColorPurple.png)
