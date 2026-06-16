#ifndef MP3PLAYER_IVISUALIZATIONSOURCE_H
#define MP3PLAYER_IVISUALIZATIONSOURCE_H
#include <vector>

class IVisualizationSource
{
    public:
        virtual ~IVisualizationSource() = default;
        virtual std::vector<float> getSpectrumBars() const = 0;
        virtual std::vector<float> getSpectrumMagnitudes() const = 0;
        virtual float getPlaybackProgress() const = 0;
        virtual float getBpmLoadFactor() const = 0;
};
#endif // MP3PLAYER_IVISUALIZATIONSOURCE_H
