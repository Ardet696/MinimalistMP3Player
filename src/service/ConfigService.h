#ifndef MP3PLAYER_CONFIGSERVICE_H
#define MP3PLAYER_CONFIGSERVICE_H

#include "IConfigService.h"
#include <filesystem>

class ConfigService : public IConfigService {
public:
    ConfigService();

    std::string getRootPath() const override;
    void setRootPath(const std::string& path) override;

private:
    std::filesystem::path configDir_;
    std::filesystem::path configFile_;

    void ensureConfigDir() const;
};

#endif // MP3PLAYER_CONFIGSERVICE_H
