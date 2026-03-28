#ifndef MP3PLAYER_ICONFIGSERVICE_H
#define MP3PLAYER_ICONFIGSERVICE_H

#include <string>

class IConfigService {
public:
    virtual ~IConfigService() = default;
    virtual std::string getRootPath() const = 0;
    virtual void setRootPath(const std::string& path) = 0;
    virtual int getTheme() const = 0;
    virtual void setTheme(int theme) = 0;
    virtual int getVisual() const = 0;
    virtual void setVisual(int visual) = 0;
};

#endif // MP3PLAYER_ICONFIGSERVICE_H
