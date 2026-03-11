#include "ConfigService.h"
#include <fstream>
#include <cstdlib>

ConfigService::ConfigService() {
#ifdef _WIN32
    const char* appData = std::getenv("APPDATA");
    configDir_ = appData ? std::filesystem::path(appData) / "minimalmp3"
                         : std::filesystem::path(".");
#else
    const char* home = std::getenv("HOME");
    configDir_ = home ? std::filesystem::path(home) / ".config" / "minimalmp3"
                      : std::filesystem::path(".");
#endif
    configFile_ = configDir_ / "config.txt";
}

std::string ConfigService::getRootPath() const {
    if (!std::filesystem::exists(configFile_)) {
        return "";
    }

    std::ifstream file(configFile_);
    std::string line;
    while (std::getline(file, line)) {
        if (line.starts_with("root_path=")) {
            return line.substr(10);  // length of "root_path="
        }
    }
    return "";
}

void ConfigService::setRootPath(const std::string& path) {
    ensureConfigDir();

    std::ofstream file(configFile_, std::ios::trunc);
    file << "root_path=" << path << "\n";
}

void ConfigService::ensureConfigDir() const {
    if (!std::filesystem::exists(configDir_)) {
        std::filesystem::create_directories(configDir_);
    }
}
