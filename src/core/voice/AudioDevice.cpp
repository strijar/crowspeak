#include "voice/AudioDevice.h"

namespace handheld::voice {
bool AudioDevice::prepare(uint8_t, bool) { return false; }
bool AudioDevice::capture(bool) { return false; }
size_t AudioDevice::read(int16_t*, size_t) { return 0; }
size_t AudioDevice::write(const int16_t*, size_t) { return 0; }
bool AudioDevice::drain(bool (*)(void*), void*) { return false; }
void AudioDevice::flush() {}
void AudioDevice::end() {}
}
