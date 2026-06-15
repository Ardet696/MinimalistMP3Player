cmake_minimum_required(VERSION 3.20)
project(MP3Player VERSION 1.0.4 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

set(FTXUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftxui)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(FTXUI_PLATFORM "linux-x86_64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(FTXUI_PLATFORM "darwin-arm64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(FTXUI_PLATFORM "darwin-x86_64")
else()
    set(FTXUI_PLATFORM "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(FTXUI_LIB_DIR ${FTXUI_DIR}/lib/${FTXUI_PLATFORM})

if(NOT EXISTS ${FTXUI_LIB_DIR}/libftxui-screen.a)
    message(FATAL_ERROR
        "Pre-built FTXUI libraries not found for ${FTXUI_PLATFORM}.\n"
        "Run: ./scripts/build-ftxui.sh\n"
        "Then re-run cmake.")
endif()

add_library(ftxui-screen STATIC IMPORTED)
set_target_properties(ftxui-screen PROPERTIES
    IMPORTED_LOCATION ${FTXUI_LIB_DIR}/libftxui-screen.a
    INTERFACE_INCLUDE_DIRECTORIES ${FTXUI_DIR}/include
)

add_library(ftxui-dom STATIC IMPORTED)
set_target_properties(ftxui-dom PROPERTIES
    IMPORTED_LOCATION ${FTXUI_LIB_DIR}/libftxui-dom.a
    INTERFACE_LINK_LIBRARIES ftxui-screen
)

add_library(ftxui-component STATIC IMPORTED)
set_target_properties(ftxui-component PROPERTIES
    IMPORTED_LOCATION ${FTXUI_LIB_DIR}/libftxui-component.a
    INTERFACE_LINK_LIBRARIES ftxui-dom
)

find_package(SDL2 QUIET CONFIG)
if(NOT SDL2_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
endif()

add_executable(MP3Player
    src/main.cpp
    src/tui/ui.cpp
    src/tui/FileManager.cpp
    src/tui/VisualsBase.cpp
    src/tui/Visuals.cpp
    src/tui/VisualsMirrored.cpp
    src/tui/VisualsRolling.cpp
    src/tui/VisualsCircular.cpp
    src/tui/VisualsPanel.cpp
    src/tui/PlayingBar.cpp
    src/tui/UserInputs.cpp

    src/library/AlbumInfo.cpp
    src/library/LibraryScanner.cpp
    src/library/Playlist.cpp
    src/library/Song.cpp
    src/library/Album.cpp
    src/library/MusicLibrary.cpp

    src/player/SongQueue.cpp
    src/player/PlaybackEngine.cpp
    src/player/DecodeThread.cpp
    src/player/AutoAdvanceManager.cpp
    src/player/PlaybackController.cpp

    src/audio/SdlAudioSink.cpp

    src/decode/Mp3Decoder.cpp
    src/decode/SpectrumAnalyzer.cpp
    src/decode/BpmDetector.cpp
    src/decode/Fft.cpp

    src/util/PathValidator.cpp

    src/events/PlaybackEventPublisher.cpp

    src/service/LibraryService.cpp
    src/service/ConfigService.cpp
    src/service/AsyncLibraryService.cpp
    src/commands/CommandQueue.cpp
)

target_compile_features(MP3Player PRIVATE cxx_std_20)
set_target_properties(MP3Player PROPERTIES CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
target_compile_options(MP3Player PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    # Security hardening (non-debug builds parse untrusted MP3 input)
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang,AppleClang>,$<NOT:$<CONFIG:Debug>>>:-fstack-protector-strong;-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=2>
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(MP3Player PRIVATE -pie -Wl,-z,relro,-z,now)
endif()

if(TARGET SDL2::SDL2)
    target_link_libraries(MP3Player PRIVATE SDL2::SDL2)
else()
    target_link_libraries(MP3Player PRIVATE PkgConfig::SDL2)
endif()
target_link_libraries(MP3Player PRIVATE
    ftxui-component
    ftxui-dom
    ftxui-screen
)

install(TARGETS MP3Player RUNTIME DESTINATION bin)

option(BUILD_TESTING "Build unit tests" OFF)

if(BUILD_TESTING)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    add_library(mp3player_core STATIC
        src/decode/Fft.cpp
        src/decode/SpectrumAnalyzer.cpp
        src/decode/BpmDetector.cpp
        src/decode/Mp3Decoder.cpp
        src/util/PathValidator.cpp
        src/service/ConfigService.cpp
        src/events/PlaybackEventPublisher.cpp
    )
    target_compile_features(mp3player_core PUBLIC cxx_std_20)
    set_target_properties(mp3player_core PROPERTIES CXX_EXTENSIONS OFF)

    add_executable(unit_tests
        tests/unit/RingBufferTest.cpp
        tests/unit/FftTest.cpp
        tests/unit/SpectrumAnalyzerTest.cpp
        tests/unit/BpmDetectorTest.cpp
        tests/unit/PathValidatorTest.cpp
        tests/unit/ConfigServiceTest.cpp
    )
    target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain mp3player_core)
    target_compile_features(unit_tests PRIVATE cxx_std_20)
    target_compile_options(unit_tests PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    )

    add_executable(stress_tests
        tests/stress/RingBufferStressTest.cpp
    )
    target_link_libraries(stress_tests PRIVATE Catch2::Catch2WithMain mp3player_core)
    target_compile_features(stress_tests PRIVATE cxx_std_20)

    add_executable(integration_tests
        tests/integration/Mp3DecoderTest.cpp
    )
    target_link_libraries(integration_tests PRIVATE Catch2::Catch2WithMain mp3player_core)
    target_compile_features(integration_tests PRIVATE cxx_std_20)
    target_compile_definitions(integration_tests PRIVATE
        FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
    )

    add_test(NAME unit COMMAND unit_tests)
    add_test(NAME stress COMMAND stress_tests)
    add_test(NAME integration COMMAND integration_tests)
endif()

option(BUILD_FUZZERS "Build libFuzzer targets (requires Clang)" OFF)

if(BUILD_FUZZERS)
    add_executable(fuzz_mp3 tests/fuzz/FuzzMp3.cpp)
    target_compile_features(fuzz_mp3 PRIVATE cxx_std_20)
    target_compile_options(fuzz_mp3 PRIVATE -g -O1 -fsanitize=fuzzer,address,undefined)
    target_link_options(fuzz_mp3 PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

option(BUILD_BENCHMARKS "Build benchmark executables" OFF)

if(BUILD_BENCHMARKS)
    function(add_bench target)
        add_executable(${target} ${ARGN})
        target_compile_features(${target} PRIVATE cxx_std_20)
        set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
        target_compile_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -O2>
        )
    endfunction()

    add_bench(CommandQueueBench
        tests/bench/CommandQueueBench.cpp
        src/commands/CommandQueue.cpp
    )

    add_bench(RingBufferBench
        tests/bench/RingBufferBench.cpp
    )

    add_bench(SpectrumBench
        tests/bench/SpectrumBench.cpp
        src/decode/SpectrumAnalyzer.cpp
        src/decode/Fft.cpp
    )

    add_bench(BpmDetectorBench
        tests/bench/BpmDetectorBench.cpp
        src/decode/BpmDetector.cpp
    )
endif()
