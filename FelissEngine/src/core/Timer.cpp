#include "Timer.h"

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
namespace { static double s_freqInv = 0.0; }
#else
#  include <time.h>
#endif

namespace Feliss {

f64 Timer::now() {
#if defined(_WIN32)
    if (s_freqInv == 0.0) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        s_freqInv = 1.0 / static_cast<double>(freq.QuadPart);
    }
    LARGE_INTEGER ctr;
    QueryPerformanceCounter(&ctr);
    return static_cast<double>(ctr.QuadPart) * s_freqInv;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<f64>(ts.tv_sec) + static_cast<f64>(ts.tv_nsec) * 1e-9;
#endif
}

Timer::Timer() { reset(); }
void Timer::reset()         { m_start = now(); }
f64  Timer::elapsed()  const { return now() - m_start; }
f32  Timer::elapsedF() const { return static_cast<f32>(elapsed()); }

DeltaTimer::DeltaTimer() : m_last(Timer::now()) {}

void DeltaTimer::tick() {
    f64 cur  = Timer::now();
    m_dt     = cur - m_last;
    m_last   = cur;
    m_total += m_dt;
    ++m_frame;

    m_fpsAcc += m_dt;
    ++m_fpsN;
    if (m_fpsAcc >= 0.5) {
        m_fps    = static_cast<f32>(m_fpsN / m_fpsAcc);
        m_fpsAcc = 0.0;
        m_fpsN   = 0;
    }
}

} // namespace Feliss
