#ifndef MP3PLAYER_PLAYBACKENGINE_H
#define MP3PLAYER_PLAYBACKENGINE_H

#include <filesystem>
#include <memory>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include "../decode/Mp3Decoder.h"
#include "../audio/SdlAudioSink.h"
#include "../util/RingBuffer.h"
#include "../config/Config.h"
#include "DecodeThread.h"
#include "../decode/SpectrumAnalyzer.h"
#include "../decode/BpmDetector.h"

/**
 * PlaybackEngine - Controller for MP3 files.
 *
 * Manages the complete playback pipeline:
 *   MP3 File -> Mp3Decoder -> DecodeThread -> RingBuffer -> SdlAudioSink -> Speakers
 */
class PlaybackEngine {
public:
    enum class State {
        Stopped,    // No file loaded or playback stopped
        Playing,
        Paused
    };

    PlaybackEngine();
    ~PlaybackEngine();

    PlaybackEngine(const PlaybackEngine&) = delete;
    PlaybackEngine& operator=(const PlaybackEngine&) = delete;
    PlaybackEngine(PlaybackEngine&&) = delete;
    PlaybackEngine& operator=(PlaybackEngine&&) = delete;

    /**
     * Load an MP3 file for playback.
     * Stops any current playback and loads the new file.
     *
     * @param mp3File - Raw path to the MP3 file
     */
    bool load(const std::filesystem::path& mp3File);
    void play();
    void pause();
    void stop();

    // State queries that can be cancelled from any thread.
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;
    State getState() const;

    /**
     * Check if playback has reached end of stream.
     * @return true if enough silent callbacks have occurred to indicate end of stream
     */
    bool hasReachedEndOfStream() const;

    std::filesystem::path getCurrentFile() const;
    int getSampleRate() const;
    int getChannels() const;

    SpectrumAnalyzer& getSpectrumAnalyzer() { return spectrumAnalyzer_; }
    float getPlaybackProgress() const;
    float getBpmLoadFactor() const;

    void setVolume(int percent);  // 0-100
    int  getVolume() const;

    /// Set the preferred output device name. Takes effect on next load().
    void setOutputDevice(const std::string& deviceName);
    std::string getOutputDevice() const;

    /// List available output devices (delegates to SdlAudioSink).
    static std::vector<std::string> listOutputDevices();

private:
    /**
     * Internal method to start the playback pipeline.
     * Must be called with mutex_ held.
     */
    bool startPlayback();

    /**
     * Internal method to stop the playback pipeline.
     * Must be called with mutex_ held.
     */
    void stopPlayback();

    // Playback components (owned by PlaybackEngine)
    std::unique_ptr<Mp3Decoder> decoder_;
    std::unique_ptr<SdlAudioSink> sink_;
    std::unique_ptr<RingBuffer<int16_t>> ringBuffer_;
    std::unique_ptr<DecodeThread> decodeThread_;

    // State management
    std::atomic<State> state_;
    std::filesystem::path currentFile_;

    // Audio format info
    int sampleRate_;
    int channels_;

    // Thread synchronization
    mutable std::mutex mutex_;  // Protects all operations

    // End-of-stream detection
    std::atomic<int> silentCallbacks_;

    // Spectrum visualization
    SpectrumAnalyzer spectrumAnalyzer_;

    // BPM detection
    BpmDetector bpmDetector_;

    // Playback progress tracking
    std::atomic<std::uint64_t> samplesPlayed_;
    std::uint64_t totalSamples_;

    // Preferred output device (empty = system default)
    std::string outputDeviceName_;

    // Volume persisted across sink recreations
    std::atomic<int> volume_{100};
};

#endif // MP3PLAYER_PLAYBACKENGINE_H
