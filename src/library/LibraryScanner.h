#ifndef MP3PLAYER_LIBRARYSCANNER_H
#define MP3PLAYER_LIBRARYSCANNER_H

#include <filesystem>
#include <string>
#include <vector>
#include "MusicLibrary.h"

class LibraryScanner {
public:
        struct Options {
                bool recursive_albums = false;
                std::vector<std::string> extensions { ".mp3"};
                bool include_hidden = false;
                bool sort_results = true;
        };
        LibraryScanner() = default;
        explicit LibraryScanner(Options& options) : options_(std::move(options)) {}

        /**
         * Scan the given root directory and return a populated MusicLibrary.
         *
         * @param rootDir  Root directory containing album folders (e.g., C:\Users\name\Music).
         * @return         MusicLibrary containing albums and songs with minimal metadata (paths/names).
         *
         * Throws:
         * - std::filesystem::filesystem_error on I/O errors (implementation may also choose to swallow
         *   errors and skip problematic entries; decide in .cpp).
         */
MusicLibrary scanRoot(const std::filesystem::path& rootDir) const;

        /**
         * Scan user's default Music directory (platform-dependent).
         * Windows: %USERPROFILE%\Music
         * Unix/Mac: ~/Music
         */
MusicLibrary scanUserMusicDir() const;

        /**
         * Prompt user for Music directory path and scan it.
         * Validates that the directory exists before scanning.
         *
         * @return MusicLibrary populated from user-provided directory
         */
static MusicLibrary scanWithPrompt();

        /**
         * Validate and scan a user-provided path string.
         * Performs security validation on the input before scanning.
         *
         * @param userInput Raw user input string (path to music directory)
         * @param outError Optional output parameter for error message if validation fails
         * @return MusicLibrary populated from validated path, or empty library on failure
         */
MusicLibrary scanFromUserInput(const std::string& userInput,
                                                      std::string* outError = nullptr) const;
const Options& options() const { return options_; }
        void setOptions(Options options) { options_ = std::move(options); }

private:
        Options options_;


};
#endif