#include "ConfigService.h"
#include <fstream>
#include <cstdlib>
#include <map>

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

std::string ConfigService::getValue(const std::string& key) const {
    if (!std::filesystem::exists(configFile_)) return "";
    std::ifstream file(configFile_);
    std::string line;
    std::string prefix = key + "=";
    while (std::getline(file, line)) {
        if (line.starts_with(prefix)) {
            return line.substr(prefix.size());
        }
    }
    return "";
}

void ConfigService::setValue(const std::string& key, const std::string& value) {
    ensureConfigDir();

    // Read all existing key-value pairs
    std::map<std::string, std::string> entries;
    if (std::filesystem::exists(configFile_)) {
        std::ifstream in(configFile_);
        std::string line;
        while (std::getline(in, line)) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                entries[line.substr(0, pos)] = line.substr(pos + 1);
            }
        }
    }

    // Update the target key
    entries[key] = value;

    // Write everything back
    std::ofstream out(configFile_, std::ios::trunc);
    for (const auto& [k, v] : entries) {
        out << k << "=" << v << "\n";
    }
}

std::string ConfigService::getRootPath() const { return getValue("root_path"); }
void ConfigService::setRootPath(const std::string& path) { setValue("root_path", path); }

int ConfigService::getTheme() const {
    std::string val = getValue("theme");
    return val.empty() ? 0 : std::stoi(val);
}
void ConfigService::setTheme(int theme) { setValue("theme", std::to_string(theme)); }

int ConfigService::getVisual() const {
    std::string val = getValue("visual");
    return val.empty() ? 0 : std::stoi(val);
}
void ConfigService::setVisual(int visual) { setValue("visual", std::to_string(visual)); }

void ConfigService::ensureConfigDir() const {
    if (!std::filesystem::exists(configDir_)) {
        std::filesystem::create_directories(configDir_);
    }
}
