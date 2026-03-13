/*
 * example_extension.cpp — Feliss Engine extension template
 * Build as shared library (.dll / .so / .dylib)
 * Link: feliss_core
 */
#define FELISS_EXPORT
#include "feliss/feliss_api.h"
#include <cstdio>

// ---- Extension metadata ----
static FlsExtMeta g_meta = {
    /* id          */ "com.example.demo_ext",
    /* name        */ "Demo Extension",
    /* description */ "Example extension showing the Feliss plugin API",
    /* author      */ "Your Name",
    /* verMaj      */ 0,
    /* verMin      */ 1,
    /* verPat      */ 0,
};

static FlsEngine g_engine = nullptr;
static float     g_elapsed = 0.0f;

// ---- Required exports ----

FLS_API FlsExtMeta* fls_ext_meta() {
    return &g_meta;
}

FLS_API FlsStatus fls_ext_init(FlsEngine e) {
    g_engine = e;
    fls_log(FLS_LOG_INFO, "DemoExt", "Demo extension initialized");
    return FLS_OK;
}

FLS_API void fls_ext_shutdown(FlsEngine /*e*/) {
    fls_log(FLS_LOG_INFO, "DemoExt", "Demo extension shutdown");
    g_engine = nullptr;
}

// ---- Optional exports ----

FLS_API void fls_ext_update(FlsEngine e, float dt) {
    g_elapsed += dt;
    // Log a message every 10 seconds
    static float next = 10.0f;
    if (g_elapsed >= next) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Extension alive — elapsed %.1f s", g_elapsed);
        fls_log(FLS_LOG_DEBUG, "DemoExt", buf);
        next += 10.0f;
    }
}

FLS_API void fls_ext_imgui(FlsEngine /*e*/) {
    // ImGui calls go here — only compiled if your project links ImGui
    // Example (uncomment when ImGui is available):
    // ImGui::Begin("Demo Extension");
    // ImGui::Text("Elapsed: %.1f s", g_elapsed);
    // ImGui::End();
}
