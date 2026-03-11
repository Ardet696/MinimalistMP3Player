#include "FastLibraryScanner.h"
#include <iostream>

std::vector<AlbumInfo> FastLibraryScanner::scanAlbums(const std::filesystem::path& rootDir) {
    std::vector<AlbumInfo> albums;

    if (!std::filesystem::exists(rootDir) || !std::filesystem::is_directory(rootDir)) {
        std::cerr << "Music directory does not exist: " << rootDir << "\n";
        return albums;
    }

    // Scan top-level directories ONLY (no file I/O yet!)
    for (const auto& entry : std::filesystem::directory_iterator(rootDir)) {
        if (!entry.is_directory()) {
            continue; // Skip files
        }

        const std::filesystem::path dirPath = entry.path();
        const std::string dirName = dirPath.filename().string();

        // Skip hidden directories
        if (dirName[0] == '.') {
            continue;
        }

        // Quick validation (checks for subdirectories and MP3 files)
        if (isValidAlbumDirectory(dirPath)) {
            // Create AlbumInfo (INSTANT - no file loading!)
            albums.emplace_back(dirPath);
        }
    }

    return albums;
}

bool FastLibraryScanner::isValidAlbumDirectory(const std::filesystem::path& dirPath) {
    bool hasMp3Files = false;
    bool hasSubdirectories = false;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_directory()) {
                hasSubdirectories = true;
                break; // Reject immediately if subdirectories found
            }
            if (entry.is_regular_file() && entry.path().extension() == ".mp3") {
                hasMp3Files = true;
            }
        }
    } catch (const std::exception& e) {
        return false;
    }

    // Valid only if: has MP3s AND no subdirectories
    return hasMp3Files && !hasSubdirectories;
}
