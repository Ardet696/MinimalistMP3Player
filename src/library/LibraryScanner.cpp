#include "LibraryScanner.h"
#include "../util/PathValidator.h"
#include <iostream>
#include <cstdlib>

MusicLibrary LibraryScanner::scanRoot(const std::filesystem::path& rootDir) const {
    MusicLibrary library;

    if (!std::filesystem::exists(rootDir) || !std::filesystem::is_directory(rootDir)) {
        std::cerr << "Root directory does not exist or is not a directory: " << rootDir << "\n";
        return library;
    }

    std::cerr << "Scanning music library: " << rootDir << "\n";

    // Iterate through top level directories
    for (const auto& entry : std::filesystem::directory_iterator(rootDir)) {
        if (!entry.is_directory()) {
            continue;
        }

        if (!options_.include_hidden && entry.path().filename().string()[0] == '.') {
            continue;
        }

        const std::filesystem::path& dirPath = entry.path();
        const std::string dirName = dirPath.filename().string();

        std::cerr << "  Scanning: " << dirName << "...\n";

        bool hasMp3Files = false;
        bool hasSubdirectories = false;

        try {
            for (const auto& innerEntry : std::filesystem::directory_iterator(dirPath)) {
                if (innerEntry.is_directory()) {
                    hasSubdirectories = true;
                }
                if (innerEntry.is_regular_file() && innerEntry.path().extension() == ".mp3") {
                    hasMp3Files = true;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "    Error scanning directory: " << e.what() << "\n";
            continue;
        }

        // Discard directories with subdirectories
        if (hasSubdirectories) {
            std::cerr << "    Skipping (contains subdirectories)\n";
            continue;
        }
        if (!hasMp3Files) {
            std::cerr << "    Skipping (no MP3 files found)\n";
            continue;
        }

        // Decide if it's an album or playlist based on naming convention for now, treat all as albums
        if (dirName.find("Playlist") != std::string::npos || dirName.find("playlist") != std::string::npos ||
         dirName.find("Mix") != std::string::npos) {

            try {
                Playlist playlist(dirPath); // Create playlist
                std::cerr << "    Added Playlist: " << playlist.getTitle()
                          << " (" << playlist.getNumSongs() << " songs)\n";
                library.addPlaylist(std::move(playlist));
            } catch (const std::exception& e) {
                std::cerr << "    Failed to create playlist: " << e.what() << "\n";
            }
        } else {
            try {
                Album album(dirPath);
                std::cerr << "    Added Album: " << album.getTitle()
                          << " (" << album.getTypeAsString() << ", "
                          << album.getNumSongs() << " songs)\n";
                library.addAlbum(std::move(album));
            } catch (const std::exception& e) {
                std::cerr << "    Failed to create album: " << e.what() << "\n";
            }
        }
    }

    std::cerr << "\nScan complete!\n";
    return library;
}

MusicLibrary LibraryScanner::scanUserMusicDir() const {

    std::filesystem::path musicDir; // User's music directory based on platform

#ifdef _WIN32
    // Windows: %USERPROFILE%\Music
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile) {
        musicDir = std::filesystem::path(userProfile) / "Music";
    }
#else
    // Unix/Linux/Mac: ~/Music
    const char* home = std::getenv("HOME");
    if (home) {
        musicDir = std::filesystem::path(home) / "Music";
    }
#endif

    if (musicDir.empty()) {
        std::cerr << "Could not determine user Music directory\n";
        return MusicLibrary();
    }

    return scanRoot(musicDir);
}


MusicLibrary LibraryScanner::scanFromUserInput(const std::string& userInput,
                                                std::string* outError) const {
    // Create validator with security-focused options
    PathValidator::Options validatorOptions;
    validatorOptions.requireExists = true;
    validatorOptions.requireDirectory = true;
    validatorOptions.allowRelativePaths = false;
    validatorOptions.strictMode = true;

    PathValidator validator(validatorOptions);
    PathValidator::ValidationResult result = validator.validate(userInput);

    if (!result.valid) {
        if (outError) {
            *outError = result.errorMessage;
        }
        std::cerr << "Path validation failed: " << result.errorMessage << "\n";
        return {};
    }

    // Path is validated and sanitized - safe to use
    return scanRoot(std::filesystem::path(result.sanitizedPath));
}
