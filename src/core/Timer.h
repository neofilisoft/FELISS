#pragma once
#include "feliss/Types.h"

namespace Feliss {

// =====================================================================
// Timer — High-resolution monotonic clock
// =====================================================================
class Timer {
public:
    Timer();

    void   reset();
    f64    elapsed() const;        // seconds since reset
    f32    elapsedF() const;       // float version

    static f64 now();              // seconds since epoch (monotonic)

private:
    f64 m_start = 0.0;
};

// =====================================================================
// DeltaTimer — Computes per-frame dt and fps
// =====================================================================
class DeltaTimer {
public:
    DeltaTimer();

    void  tick();                  // call once per frame
    f64   deltaTime()   const { return m_dt; }
    f64   totalTime()   const { return m_total; }
    f32   fps()         const { return m_fps; }
    u64   frameCount()  const { return m_frame; }

private:
    f64 m_last   = 0.0;
    f64 m_dt     = 0.0;
    f64 m_total  = 0.0;
    f32 m_fps    = 0.0f;
    f64 m_fpsAcc = 0.0;
    u32 m_fpsN   = 0;
    u64 m_frame  = 0;
};

} // namespace Feliss
