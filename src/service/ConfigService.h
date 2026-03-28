#ifndef MP3PLAYER_CONFIGSERVICE_H
#define MP3PLAYER_CONFIGSERVICE_H

#include "IConfigService.h"
#include <filesystem>

class ConfigService : public IConfigService {
public:
    ConfigService();

    std::string getRootPath() const override;
    void setRootPath(const std::string& path) override;
    int getTheme() const override;
    void setTheme(int theme) override;
    int getVisual() const override;
    void setVisual(int visual) override;

private:
    std::filesystem::path configDir_;
    std::filesystem::path configFile_;

    void ensureConfigDir() const;
    std::string getValue(const std::string& key) const;
    void setValue(const std::string& key, const std::string& value);
};

#endif // MP3PLAYER_CONFIGSERVICE_H
