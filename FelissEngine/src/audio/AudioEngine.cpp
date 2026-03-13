#include "audio/AudioEngine.h"
#include "core/Logger.h"

namespace Feliss {

AudioEngine::AudioEngine()  = default;
AudioEngine::~AudioEngine() { shutdown(); }

bool AudioEngine::init() {
    // TODO: Initialize OpenAL or miniaudio context
    FLS_INFO("Audio", "AudioEngine initialized (stub — link OpenAL/miniaudio)");
    m_initialized = true;
    return true;
}

void AudioEngine::shutdown() {
    if (!m_initialized) return;
    m_initialized = false;
    FLS_INFO("Audio", "AudioEngine shutdown");
}

void AudioEngine::update(f32)              {}
void AudioEngine::setListenerPos(Vec3)     {}
void AudioEngine::setListenerOrientation(Vec3, Vec3) {}
void AudioEngine::setMasterVolume(f32 v)  { m_masterVolume = v; }

AssetID AudioEngine::loadClip(const std::string& path) {
    FLS_INFOF("Audio", "LoadClip (stub): " << path);
    return NULL_ASSET;
}
void AudioEngine::unloadClip(AssetID)          {}
void* AudioEngine::createSource()              { return nullptr; }
void  AudioEngine::destroySource(void*)        {}
void  AudioEngine::playSource(void*,AssetID,bool) {}
void  AudioEngine::stopSource(void*)           {}
void  AudioEngine::pauseSource(void*)          {}
void  AudioEngine::setSourceVolume(void*,f32)  {}
void  AudioEngine::setSourcePitch(void*,f32)   {}
void  AudioEngine::setSourcePos(void*,Vec3)    {}
bool  AudioEngine::isSourcePlaying(void*) const { return false; }

} // namespace Feliss
