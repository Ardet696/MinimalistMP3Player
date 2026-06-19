#include "SdlAudioSubsystem.h"

#include <SDL.h>

SdlAudioSubsystem::SdlAudioSubsystem() : ok_(SDL_InitSubSystem(SDL_INIT_AUDIO) == 0) {}

SdlAudioSubsystem::~SdlAudioSubsystem() {
    if (ok_) SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
