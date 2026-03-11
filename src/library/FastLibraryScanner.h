#ifndef MP3PLAYER_FASTLIBRARYSCANNER_H
#define MP3PLAYER_FASTLIBRARYSCANNER_H

#include <filesystem>
#include <vector>
#include "AlbumInfo.h"

/**
 * FastLibraryScanner - Blazing fast directory scanner.
 *
 *   - Only scan directory NAMES (no file I/O)
 *   - Return lightweight AlbumInfo objects (no Song loading)
 *   - Entire scan should take < 100ms even for 1000 albums
 */
class FastLibraryScanner {
public:
    /**
     * Scan Music directory and return list of albums.
     * FAST - only reads directory names, doesn't open any files!
     *
     * @param rootDir - Path to Music directory
     * @return Vector of AlbumInfo objects (lightweight, instant construction)
     */
    static std::vector<AlbumInfo> scanAlbums(const std::filesystem::path& rootDir);

private:
    /**
     * Check if directory is valid for scanning.
     * Rejects directories with subdirectories or no MP3 files.
     */
    static bool isValidAlbumDirectory(const std::filesystem::path& dirPath);
};

#endif
