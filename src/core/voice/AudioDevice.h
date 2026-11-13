#pragma once
#include <cstddef>
#include <cstdint>
#include "voice/VoiceTypes.h"

namespace handheld::voice {
// CrowPanel's current radio wiring does not expose an audio backend.
class AudioDevice {
public:
    bool prepare(uint8_t volume, bool record = false);
    bool capture(bool enabled);
    size_t read(int16_t* pcm, size_t samples);
    size_t write(const int16_t* pcm, size_t samples);
    uint32_t playedSamples() const { return 0; }
    bool drain(bool (*cancelled)(void*) = nullptr, void* context = nullptr);
    void volume(uint8_t) {}
    void end();
    void flush();
    Code error() const { return Code::AudioUnavailable; }
};
}
