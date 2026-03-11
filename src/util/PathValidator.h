#ifndef MP3PLAYER_PATHVALIDATOR_H
#define MP3PLAYER_PATHVALIDATOR_H

#include <string>
#include <string_view>
#include <filesystem>
#include <optional>

/**
 * PathValidator - Security-focused input validation for filesystem paths.
 */
class PathValidator {
public:
    static constexpr size_t DefaultMaxPathLength = 4096;

    // ASCII control character
    static constexpr unsigned char AsciiPrintableMin = 32;  // Space
    static constexpr unsigned char AsciiTab = 9;            // Tab
    static constexpr unsigned char AsciiDel = 127;          // DEL
    static constexpr unsigned char Utf8AsciiMax = 0x7F;
    static constexpr unsigned char Utf8TwoBytePrefix = 0xC0;
    static constexpr unsigned char Utf8TwoByteMask = 0xE0;
    static constexpr unsigned char Utf8ThreeBytePrefix = 0xE0;
    static constexpr unsigned char Utf8ThreeByteMask = 0xF0;
    static constexpr unsigned char Utf8FourBytePrefix = 0xF0;
    static constexpr unsigned char Utf8FourByteMask = 0xF8;
    static constexpr unsigned char Utf8ContinuationPrefix = 0x80;
    static constexpr unsigned char Utf8ContinuationMask = 0xC0;
    static constexpr unsigned char Utf8TwoByteMin = 0xC2;
    static constexpr unsigned char Utf8ThreeByteOverlongMin = 0xA0;
    static constexpr unsigned char Utf8FourByteOverlongMin = 0x90;
    static constexpr unsigned char Utf8FourByteMax = 0xF4;
    static constexpr unsigned char Utf8FourByteLastMax = 0x8F;

    enum class ValidationError {
        None,
        EmptyPath,
        PathTooLong,
        ContainsNullByte,
        ContainsPathTraversal,
        ContainsControlCharacters,
        ContainsShellMetacharacters,
        InvalidUtf8,
        PathDoesNotExist,
        PathNotDirectory,
        PathNotAccessible
    };

    struct ValidationResult {
        bool valid;
        ValidationError error;
        std::string sanitizedPath;
        std::string errorMessage;

        explicit operator bool() const { return valid; }
    };

    struct Options {
        size_t maxPathLength;
        bool requireExists;
        bool requireDirectory;
        bool allowRelativePaths;
        bool strictMode;

        Options()
            : maxPathLength(DefaultMaxPathLength)
            , requireExists(true)
            , requireDirectory(true)
            , allowRelativePaths(false)
            , strictMode(true) {}
    };

    explicit PathValidator(const Options& options = Options()) : options_(options) {}

    /**
     * Validate a user-provided path string.
     *
     * @param input Raw user input string
     * @return ValidationResult containing validation status and sanitized path if valid
     */
ValidationResult validate(std::string_view input) const;

    /**
     * Quick check if a path is valid without detailed error info.
     */
bool isValid(std::string_view input) const;

    /**
     * Get readable error message for a validation error.
     */
static std::string getErrorMessage(ValidationError error);

const Options& options() const { return options_; }
    void setOptions(Options options) { options_ = options; }

private:
    Options options_;

bool containsNullByte(std::string_view str) const;
bool containsPathTraversal(std::string_view str) const;
bool containsControlCharacters(std::string_view str) const;
bool containsShellMetacharacters(std::string_view str) const;
bool isValidUtf8(std::string_view str) const;
std::string trimWhitespace(std::string_view str) const;
std::string normalizePath(std::string_view str) const;
};

#endif