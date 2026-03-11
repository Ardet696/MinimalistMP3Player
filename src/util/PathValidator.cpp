#include "PathValidator.h"
#include <algorithm>
#include <cctype>

PathValidator::ValidationResult PathValidator::validate(std::string_view input) const {
    ValidationResult result{false, ValidationError::None, "", ""};

    std::string trimmed = trimWhitespace(input);

    if (trimmed.empty()) {
        result.error = ValidationError::EmptyPath;
        result.errorMessage = getErrorMessage(result.error);
        return result;
    }
    if (trimmed.length() > options_.maxPathLength) {
        result.error = ValidationError::PathTooLong;
        result.errorMessage = getErrorMessage(result.error);
        return result;
    }
    if (containsNullByte(trimmed)) {
        result.error = ValidationError::ContainsNullByte;
        result.errorMessage = getErrorMessage(result.error);
        return result;
    }
    if (containsPathTraversal(trimmed)) {
        result.error = ValidationError::ContainsPathTraversal;
        result.errorMessage = getErrorMessage(result.error);
        return result;
    }
    if (containsControlCharacters(trimmed)) {
        result.error = ValidationError::ContainsControlCharacters;
        result.errorMessage = getErrorMessage(result.error);
        return result;
    }

    if (options_.strictMode && containsShellMetacharacters(trimmed)) {
        result.error = ValidationError::ContainsShellMetacharacters;
        result.errorMessage = getErrorMessage(result.error);
        return result;
    }
    if (!isValidUtf8(trimmed)) {
        result.error = ValidationError::InvalidUtf8;
        result.errorMessage = getErrorMessage(result.error);
        return result;
    }

    std::string normalized = normalizePath(trimmed);

    if (!options_.allowRelativePaths) {
        std::filesystem::path fsPath(normalized);
        if (fsPath.is_relative()) {
            result.error = ValidationError::ContainsPathTraversal;
            result.errorMessage = "Relative paths are not allowed";
            return result;
        }
    }

    if (options_.requireExists) {
        std::error_code ec;
        if (!std::filesystem::exists(normalized, ec) || ec) {
            result.error = ValidationError::PathDoesNotExist;
            result.errorMessage = getErrorMessage(result.error);
            return result;
        }

        auto status = std::filesystem::status(normalized, ec);
        if (ec) {
            result.error = ValidationError::PathNotAccessible;
            result.errorMessage = getErrorMessage(result.error);
            return result;
        }

        if (options_.requireDirectory) {
            if (!std::filesystem::is_directory(status)) {
                result.error = ValidationError::PathNotDirectory;
                result.errorMessage = getErrorMessage(result.error);
                return result;
            }
        }
    }

    result.valid = true;
    result.error = ValidationError::None;
    result.sanitizedPath = normalized;
    return result; // GO!
}

bool PathValidator::isValid(std::string_view input) const {
    return validate(input).valid;
}

std::string PathValidator::getErrorMessage(ValidationError error) {
    switch (error) {
        case ValidationError::None:
            return "No error";
        case ValidationError::EmptyPath:
            return "Path cannot be empty";
        case ValidationError::PathTooLong:
            return "Path exceeds maximum allowed length";
        case ValidationError::ContainsNullByte:
            return "Path contains invalid null byte";
        case ValidationError::ContainsPathTraversal:
            return "Path contains invalid traversal sequence";
        case ValidationError::ContainsControlCharacters:
            return "Path contains invalid control characters";
        case ValidationError::ContainsShellMetacharacters:
            return "Path contains potentially dangerous characters";
        case ValidationError::InvalidUtf8:
            return "Path contains invalid UTF-8 encoding";
        case ValidationError::PathDoesNotExist:
            return "Path does not exist";
        case ValidationError::PathNotDirectory:
            return "Path is not a directory";
        case ValidationError::PathNotAccessible:
            return "Path is not accessible";
        default:
            return "Unknown validation error";
    }
}

bool PathValidator::containsNullByte(std::string_view str) const {
    return str.find('\0') != std::string_view::npos;
}

bool PathValidator::containsPathTraversal(std::string_view str) const {

    if (str.find("..") != std::string_view::npos) {
        return true;
    }

    // URL-encoded traversal attempts
    if (str.find("%2e%2e") != std::string_view::npos ||
        str.find("%2E%2E") != std::string_view::npos) {
        return true;
    }

    // Double-encoded attempts
    if (str.find("%252e") != std::string_view::npos ||
        str.find("%252E") != std::string_view::npos) {
        return true;
    }

    // Unicode/overlong UTF-8 encoding attempts for '.'
    // %c0%ae is overlong encoding for '.'
    if (str.find("%c0%ae") != std::string_view::npos ||
        str.find("%C0%AE") != std::string_view::npos) {
        return true;
    }

    return false;
}

bool PathValidator::containsControlCharacters(std::string_view str) const {
    for (unsigned char c : str) {
        // Allow only printable ASCII and valid UTF-8 continuation bytes
        if (c < AsciiPrintableMin && c != AsciiTab) {
            return true;
        }
        // DEL character
        if (c == AsciiDel) {
            return true;
        }
    }
    return false;
}

bool PathValidator::containsShellMetacharacters(std::string_view str) const {
    // Characters that could be used for command injection or shell expansion
    // Note: We're being conservative here for security
    static constexpr std::string_view dangerous = "|;&$`\"'<>(){}[]!#*?\\~";

    // On Windows, backslash is valid path separator, so handle it specially
#ifdef _WIN32
    static constexpr std::string_view dangerousWin = "|;&$`\"'<>(){}[]!#*?~";
    for (char c : str) {
        if (dangerousWin.find(c) != std::string_view::npos) {
            return true;
        }
    }
#else
    for (char c : str) {
        if (dangerous.find(c) != std::string_view::npos) {
            return true;
        }
    }
#endif

    // Check for newlines which could be used for header injection style attacks
    if (str.find('\n') != std::string_view::npos ||
        str.find('\r') != std::string_view::npos) {
        return true;
    }

    return false;
}

bool PathValidator::isValidUtf8(std::string_view str) const {
    size_t i = 0;
    while (i < str.size()) {
        unsigned char c = static_cast<unsigned char>(str[i]);

        if (c <= Utf8AsciiMax) {
            // ASCII character
            i++;
        } else if ((c & Utf8TwoByteMask) == Utf8TwoBytePrefix) {
            // 2-byte sequence
            if (i + 1 >= str.size()) return false;
            if ((static_cast<unsigned char>(str[i + 1]) & Utf8ContinuationMask) != Utf8ContinuationPrefix) return false;
            // Check for overlong encoding
            if (c < Utf8TwoByteMin) return false;
            i += 2;
        } else if ((c & Utf8ThreeByteMask) == Utf8ThreeBytePrefix) {
            // 3-byte sequence
            if (i + 2 >= str.size()) return false;
            if ((static_cast<unsigned char>(str[i + 1]) & Utf8ContinuationMask) != Utf8ContinuationPrefix) return false;
            if ((static_cast<unsigned char>(str[i + 2]) & Utf8ContinuationMask) != Utf8ContinuationPrefix) return false;
            // Check for overlong encoding
            if (c == Utf8ThreeBytePrefix && static_cast<unsigned char>(str[i + 1]) < Utf8ThreeByteOverlongMin) return false;
            i += 3;
        } else if ((c & Utf8FourByteMask) == Utf8FourBytePrefix) {
            // 4-byte sequence
            if (i + 3 >= str.size()) return false;
            if ((static_cast<unsigned char>(str[i + 1]) & Utf8ContinuationMask) != Utf8ContinuationPrefix) return false;
            if ((static_cast<unsigned char>(str[i + 2]) & Utf8ContinuationMask) != Utf8ContinuationPrefix) return false;
            if ((static_cast<unsigned char>(str[i + 3]) & Utf8ContinuationMask) != Utf8ContinuationPrefix) return false;
            // Check for overlong encoding
            if (c == Utf8FourBytePrefix && static_cast<unsigned char>(str[i + 1]) < Utf8FourByteOverlongMin) return false;
            // Check for code points > U+10FFFF
            if (c > Utf8FourByteMax) return false;
            if (c == Utf8FourByteMax && static_cast<unsigned char>(str[i + 1]) > Utf8FourByteLastMax) return false;
            i += 4;
        } else {
            // Invalid UTF-8 start byte
            return false;
        }
    }
    return true;
}

std::string PathValidator::trimWhitespace(std::string_view str) const {
    size_t start = 0;
    size_t end = str.size();

    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }

    return std::string(str.substr(start, end - start));
}

std::string PathValidator::normalizePath(std::string_view str) const {
    std::string result(str);

#ifdef _WIN32
    std::replace(result.begin(), result.end(), '/', '\\');
#endif

    // Remove trailing slashes but keep root slash on Unix
    while (result.size() > 1 &&
           (result.back() == '/' || result.back() == '\\')) {
        result.pop_back();
    }

    return result;
}