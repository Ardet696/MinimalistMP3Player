#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build-test"
JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu)"

FUZZ_SECONDS="${FUZZ_SECONDS:-60}"

usage() {
    echo "Usage: ./scripts/test.sh [unit|stress|integration|asan|tsan|fuzz|all]"
    echo "  (no args)     Run all tests"
    echo "  unit           Unit tests only"
    echo "  stress         Stress tests only"
    echo "  integration    Integration tests only"
    echo "  asan           All tests under Address + UndefinedBehavior sanitizers"
    echo "  tsan           Stress tests under ThreadSanitizer"
    echo "  fuzz           libFuzzer smoke run over the MP3 decoder (FUZZ_SECONDS=60)"
    echo "  all            Normal + ASan/UBSan + TSan + fuzz smoke"
    exit 1
}

build_and_run() {
    local build_dir="$1"; shift
    local cmake_extra=("$@")

    cmake -B "$build_dir" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON "${cmake_extra[@]}" "$ROOT" > /dev/null
    cmake --build "$build_dir" -j"$JOBS" > /dev/null 2>&1
}

run_tests() {
    local build_dir="$1"; shift
    cd "$build_dir" && ctest --output-on-failure "$@"
}

MODE="${1:-}"

case "$MODE" in
    unit|stress|integration)
        build_and_run "$BUILD"
        run_tests "$BUILD" -R "$MODE"
        ;;
    asan)
        echo "==> ASan/UBSan build"
        build_and_run "$BUILD-asan" \
            -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -g" \
            -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
        ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 run_tests "$BUILD-asan"
        ;;
    fuzz)
        echo "==> Fuzz smoke (${FUZZ_SECONDS}s)"
        cmake -B "$BUILD-fuzz" -DCMAKE_BUILD_TYPE=Debug -DBUILD_FUZZERS=ON \
            -DCMAKE_CXX_COMPILER=clang++ "$ROOT" > /dev/null
        cmake --build "$BUILD-fuzz" --target fuzz_mp3 -j"$JOBS" > /dev/null 2>&1
        mkdir -p "$BUILD-fuzz/corpus"
        cp "$ROOT"/tests/fixtures/*.mp3 "$BUILD-fuzz/corpus/" 2>/dev/null || true
        "$BUILD-fuzz/fuzz_mp3" "$BUILD-fuzz/corpus" -max_total_time="$FUZZ_SECONDS"
        ;;
    tsan)
        echo "==> TSan build"
        build_and_run "$BUILD-tsan" \
            -DCMAKE_CXX_FLAGS="-fsanitize=thread -g" \
            -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"
        TSAN_OPTIONS=halt_on_error=1 run_tests "$BUILD-tsan" -R "stress|unit"
        ;;
    all)
        "$0" ""
        "$0" asan
        "$0" tsan
        "$0" fuzz
        ;;
    "")
        build_and_run "$BUILD"
        run_tests "$BUILD"
        ;;
    *)
        usage
        ;;
esac
