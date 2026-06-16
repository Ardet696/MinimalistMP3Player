#ifndef MP3PLAYER_ILIBRARYSERVICE_H
#define MP3PLAYER_ILIBRARYSERVICE_H
#include "ILibraryQuery.h"
#include "IPlaybackControl.h"
#include "IVisualizationSource.h"

class NotificationBus;

class ILibraryService : public ILibraryQuery,
                        public IPlaybackControl,
                        public IVisualizationSource
{
    public:
        ~ILibraryService() override = default;
        virtual NotificationBus& getNotificationBus() = 0;
};
#endif //MP3PLAYER_ILIBRARYSERVICE_H
