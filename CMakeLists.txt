cmake_minimum_required(VERSION 3.20)
project(MP3Player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization: use Release flags by default if no build type specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

# FTXUI is statically linked — don't install its headers/libs
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/FTXUI)

# Find SDL2 — try CONFIG mode first (Arch, vcpkg), fall back to MODULE (Debian/Ubuntu/Fedora)
find_package(SDL2 QUIET CONFIG)
if(NOT SDL2_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
endif()

add_executable(MP3Player
        # TUI
        src/tui/ui.cpp
        src/tui/FileManager.cpp
        src/tui/FileManager.h
        src/tui/Visuals.cpp
        src/tui/VisualsMirrored.cpp
        src/tui/VisualsRolling.cpp
        src/tui/VisualsCircular.cpp
        src/tui/VisualsPanel.cpp
        src/tui/Visuals.h
        src/tui/PlayingBar.cpp
        src/tui/PlayingBar.h
        src/tui/UserInputs.cpp
        src/tui/UserInputs.h
        # Library management
        src/library/AlbumInfo.cpp
        src/library/AlbumInfo.h
        src/library/FastLibraryScanner.cpp
        src/library/FastLibraryScanner.h
        src/library/LibraryScanner.cpp
        src/library/LibraryScanner.h
        src/library/Playlist.cpp
        src/library/Playlist.h
        src/library/Song.cpp
        src/library/Song.h
        src/library/Album.cpp
        src/library/Album.h
        src/library/MusicLibrary.cpp
        src/library/MusicLibrary.h
        # Player core
        src/player/AlbumPlayer.cpp
        src/player/AlbumPlayer.h
        src/player/SongQueue.cpp
        src/player/SongQueue.h
        src/player/PlaybackEngine.cpp
        src/player/PlaybackEngine.h
        src/player/DecodeThread.cpp
        src/player/DecodeThread.h
        src/player/AutoAdvanceManager.cpp
        src/player/AutoAdvanceManager.h
        src/player/PlaybackController.cpp
        src/player/PlaybackController.h
        # Audio
        src/audio/SdlAudioSink.cpp
        src/audio/SdlAudioSink.h
        # Decoder
        src/decode/Mp3Decoder.cpp
        src/decode/Mp3Decoder.h
        src/decode/SpectrumAnalyzer.cpp
        src/decode/SpectrumAnalyzer.h
        src/decode/BpmDetector.cpp
        src/decode/BpmDetector.h
        # Utilities
        src/util/AudioFormat.h
        src/util/RingBuffer.h
        src/util/PathValidator.cpp
        src/util/PathValidator.h
        # Config
        src/config/Config.h
        # Events
        src/events/PlaybackEvent.h
        src/events/PlaybackEventPublisher.cpp
        src/events/PlaybackEventPublisher.h
        # Service layer
        src/service/ILibraryService.h
        src/service/LibraryService.cpp
        src/service/LibraryService.h
        src/service/IConfigService.h
        src/service/ConfigService.cpp
        src/service/ConfigService.h
        src/tui/Palette.h
        src/decode/Fft.cpp
        src/decode/Fft.h
)

# Link SDL2 — use the target that was found (CONFIG vs pkg-config)
if(TARGET SDL2::SDL2)
    target_link_libraries(MP3Player PRIVATE
            SDL2::SDL2main
            SDL2::SDL2
            ftxui::screen
            ftxui::dom
            ftxui::component
    )
else()
    target_link_libraries(MP3Player PRIVATE
            PkgConfig::SDL2
            ftxui::screen
            ftxui::dom
            ftxui::component
    )
endif()

# Install
install(TARGETS MP3Player RUNTIME DESTINATION bin)
