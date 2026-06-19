#ifndef MP3PLAYER_SDLAUDIOSUBSYSTEM_H
#define MP3PLAYER_SDLAUDIOSUBSYSTEM_H

class SdlAudioSubsystem {
public:
    SdlAudioSubsystem();
    ~SdlAudioSubsystem();

    bool ok() const { return ok_; }

    SdlAudioSubsystem(const SdlAudioSubsystem&) = delete;
    SdlAudioSubsystem& operator=(const SdlAudioSubsystem&) = delete;
    SdlAudioSubsystem(SdlAudioSubsystem&&) = delete;
    SdlAudioSubsystem& operator=(SdlAudioSubsystem&&) = delete;

private:
    bool ok_ = false;
};

#endif
