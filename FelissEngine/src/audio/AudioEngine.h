#pragma once
#include "feliss/Types.h"
#include <string>

namespace Feliss {

// =====================================================================
// AudioEngine — abstraction for audio (OpenAL / miniaudio)
// =====================================================================
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool init();
    void shutdown();
    void update(f32 dt);

    // Listener (usually attached to main camera)
    void setListenerPos(Vec3 pos);
    void setListenerOrientation(Vec3 forward, Vec3 up);
    void setMasterVolume(f32 v);

    // Clip management
    AssetID loadClip(const std::string& path);
    void    unloadClip(AssetID id);

    // Playback
    void* createSource();
    void  destroySource(void* handle);
    void  playSource(void* handle, AssetID clipID, bool loop = false);
    void  stopSource(void* handle);
    void  pauseSource(void* handle);
    void  setSourceVolume(void* handle, f32 v);
    void  setSourcePitch(void* handle, f32 p);
    void  setSourcePos(void* handle, Vec3 pos);
    bool  isSourcePlaying(void* handle) const;

    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    f32  m_masterVolume = 1.0f;
    void* m_ctx = nullptr;  // OpenAL / miniaudio context
};

} // namespace Feliss
