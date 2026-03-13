#pragma once
/*
 * feliss_api.h — Feliss Engine Stable C ABI
 * =====================================================================
 * All interop (C#, Rust, Zig, Python) goes through this interface.
 * Only C linkage and POD types are used; no C++ exceptions cross the boundary.
 *
 * C# usage:
 *   [DllImport("feliss_core")]
 *   static extern IntPtr fls_engine_create(ref FlsEngineDesc desc);
 *
 * Rust usage:
 *   extern "C" { fn fls_engine_create(d: *const FlsEngineDesc) -> *mut FlsEngineOpaque; }
 * =====================================================================
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Export macros ---- */
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef FELISS_EXPORT
    #define FLS_API __declspec(dllexport)
  #else
    #define FLS_API __declspec(dllimport)
  #endif
#else
  #define FLS_API __attribute__((visibility("default")))
#endif

/* ---- Opaque handles (use void* only, never dereference externally) ---- */
typedef struct FlsEngine_s*    FlsEngine;
typedef struct FlsScene_s*     FlsScene;
typedef void*                  FlsComponent;
typedef uint64_t               FlsEntityID;
typedef uint64_t               FlsAssetID;
#define FLS_NULL_ENTITY  ((FlsEntityID)0)
#define FLS_NULL_ASSET   ((FlsAssetID)0)

/* ---- Status ---- */
typedef enum {
    FLS_OK            = 0,
    FLS_ERR           = 1,
    FLS_NOT_FOUND     = 2,
    FLS_INVALID_ARG   = 3,
    FLS_UNSUPPORTED   = 4,
    FLS_ALREADY_EXISTS= 5,
    FLS_OUT_OF_MEMORY = 6,
} FlsStatus;

/* ---- Render API ---- */
typedef enum {
    FLS_RENDER_NONE  = 0,
    FLS_RENDER_OGL   = 1,
    FLS_RENDER_VK    = 2,
    FLS_RENDER_DX11  = 3,
    FLS_RENDER_DX12  = 4,
    FLS_RENDER_METAL = 5,
    FLS_RENDER_WEBGL = 6,
} FlsRenderAPI;

/* ---- Engine mode ---- */
typedef enum { FLS_MODE_GAME=0, FLS_MODE_EDITOR=1, FLS_MODE_SERVER=2 } FlsEngineMode;

/* ---- Math PODs ---- */
typedef struct { float x,y; }       FlsVec2;
typedef struct { float x,y,z; }     FlsVec3;
typedef struct { float x,y,z,w; }   FlsVec4;
typedef struct { float x,y,z,w; }   FlsQuat;
typedef struct { float m[16]; }     FlsMat4;
typedef struct { float r,g,b,a; }   FlsColor;
typedef struct { FlsVec3 pos; FlsQuat rot; FlsVec3 scl; } FlsTransform;

/* ---- Engine descriptor ---- */
typedef struct {
    const char*   title;
    int           windowW, windowH;
    bool          fullscreen, vsync;
    FlsRenderAPI  renderAPI;
    FlsEngineMode mode;
    const char*   projectPath;
    const char*   logFile;
    bool          enableLua, enableCSharp;
    int           targetFPS;   /* 0 = unlimited */
} FlsEngineDesc;

/* ---- Version ---- */
typedef struct { uint32_t major,minor,patch; } FlsVersion;
FLS_API FlsVersion    fls_get_version(void);
FLS_API const char*   fls_version_string(void);

/* ---- Error ---- */
FLS_API const char*   fls_status_string(FlsStatus s);
FLS_API const char*   fls_last_error(void);

/* ---- Engine lifecycle ---- */
FLS_API FlsEngine     fls_engine_create(const FlsEngineDesc* desc);
FLS_API void          fls_engine_destroy(FlsEngine e);
FLS_API FlsStatus     fls_engine_init(FlsEngine e);
FLS_API void          fls_engine_run(FlsEngine e);
FLS_API void          fls_engine_stop(FlsEngine e);
FLS_API bool          fls_engine_is_running(FlsEngine e);
FLS_API double        fls_engine_delta_time(FlsEngine e);
FLS_API double        fls_engine_elapsed_time(FlsEngine e);
FLS_API float         fls_engine_fps(FlsEngine e);

/* ---- Scene ---- */
FLS_API FlsScene      fls_scene_create(FlsEngine e, const char* name);
FLS_API void          fls_scene_destroy(FlsScene s);
FLS_API FlsStatus     fls_scene_load(FlsEngine e, const char* path);
FLS_API FlsStatus     fls_scene_save(FlsScene s, const char* path);
FLS_API FlsScene      fls_scene_get_active(FlsEngine e);
FLS_API const char*   fls_scene_name(FlsScene s);

/* ---- Entity ---- */
FLS_API FlsEntityID   fls_entity_create(FlsScene s, const char* name);
FLS_API void          fls_entity_destroy(FlsScene s, FlsEntityID id);
FLS_API bool          fls_entity_valid(FlsScene s, FlsEntityID id);
FLS_API const char*   fls_entity_name(FlsScene s, FlsEntityID id);
FLS_API void          fls_entity_set_name(FlsScene s, FlsEntityID id, const char* name);
FLS_API void          fls_entity_set_active(FlsScene s, FlsEntityID id, bool active);
FLS_API bool          fls_entity_is_active(FlsScene s, FlsEntityID id);
FLS_API FlsEntityID   fls_entity_parent(FlsScene s, FlsEntityID id);
FLS_API void          fls_entity_set_parent(FlsScene s, FlsEntityID child, FlsEntityID parent);

/* ---- Transform ---- */
FLS_API FlsVec3       fls_transform_pos(FlsScene s, FlsEntityID id);
FLS_API void          fls_transform_set_pos(FlsScene s, FlsEntityID id, FlsVec3 p);
FLS_API FlsQuat       fls_transform_rot(FlsScene s, FlsEntityID id);
FLS_API void          fls_transform_set_rot(FlsScene s, FlsEntityID id, FlsQuat q);
FLS_API FlsVec3       fls_transform_scale(FlsScene s, FlsEntityID id);
FLS_API void          fls_transform_set_scale(FlsScene s, FlsEntityID id, FlsVec3 sc);
FLS_API FlsMat4       fls_transform_world_matrix(FlsScene s, FlsEntityID id);

/* ---- Component types ---- */
typedef enum {
    FLS_COMP_TRANSFORM=0, FLS_COMP_MESH, FLS_COMP_LIGHT,
    FLS_COMP_CAMERA, FLS_COMP_RIGIDBODY, FLS_COMP_COLLIDER,
    FLS_COMP_SCRIPT, FLS_COMP_AUDIO, FLS_COMP_SPRITE,
    FLS_COMP_ANIMATOR, FLS_COMP_CUSTOM=0xFF,
} FlsComponentType;

FLS_API FlsComponent  fls_component_add(FlsScene s, FlsEntityID id, FlsComponentType t);
FLS_API FlsComponent  fls_component_get(FlsScene s, FlsEntityID id, FlsComponentType t);
FLS_API bool          fls_component_has(FlsScene s, FlsEntityID id, FlsComponentType t);
FLS_API void          fls_component_remove(FlsScene s, FlsEntityID id, FlsComponentType t);

/* ---- Scripting ---- */
FLS_API FlsStatus     fls_lua_exec_file(FlsEngine e, const char* path);
FLS_API FlsStatus     fls_lua_exec_string(FlsEngine e, const char* code);
FLS_API FlsStatus     fls_csharp_load_assembly(FlsEngine e, const char* path);

/* ---- Input ---- */
FLS_API bool          fls_key_pressed(FlsEngine e, int keyCode);
FLS_API bool          fls_key_held(FlsEngine e, int keyCode);
FLS_API bool          fls_key_released(FlsEngine e, int keyCode);
FLS_API bool          fls_mouse_button(FlsEngine e, int btn);
FLS_API FlsVec2       fls_mouse_pos(FlsEngine e);
FLS_API FlsVec2       fls_mouse_delta(FlsEngine e);
FLS_API float         fls_mouse_scroll(FlsEngine e);

/* ---- Assets ---- */
FLS_API FlsAssetID    fls_asset_load(FlsEngine e, const char* path);
FLS_API void          fls_asset_unload(FlsEngine e, FlsAssetID id);
FLS_API bool          fls_asset_loaded(FlsEngine e, FlsAssetID id);
FLS_API const char*   fls_asset_path(FlsEngine e, FlsAssetID id);

/* ---- Extensions ---- */
FLS_API FlsStatus     fls_ext_load(FlsEngine e, const char* path);
FLS_API FlsStatus     fls_ext_unload(FlsEngine e, const char* id);
FLS_API void          fls_ext_enable(FlsEngine e, const char* id);
FLS_API void          fls_ext_disable(FlsEngine e, const char* id);
FLS_API bool          fls_ext_is_enabled(FlsEngine e, const char* id);
FLS_API int           fls_ext_count(FlsEngine e);
FLS_API const char*   fls_ext_id_at(FlsEngine e, int idx);

/* ---- Log ---- */
typedef enum { FLS_LOG_TRACE=0,FLS_LOG_DEBUG,FLS_LOG_INFO,FLS_LOG_WARN,FLS_LOG_ERROR,FLS_LOG_FATAL } FlsLogLevel;
typedef void (*FlsLogCb)(FlsLogLevel lvl, const char* tag, const char* msg, void* ud);
FLS_API void          fls_log(FlsLogLevel lvl, const char* tag, const char* msg);
FLS_API void          fls_log_set_callback(FlsLogCb cb, void* ud);
FLS_API void          fls_log_set_level(FlsLogLevel min);

/* ====================================================================
 * Extension ABI — implement these symbols in your .dll/.so
 * ====================================================================
 * Required exports:
 *   FlsExtMeta* fls_ext_meta(void);
 *   FlsStatus   fls_ext_init(FlsEngine);
 *   void        fls_ext_shutdown(FlsEngine);
 * Optional:
 *   void        fls_ext_update(FlsEngine, float dt);
 *   void        fls_ext_imgui(FlsEngine);
 */
typedef struct {
    const char* id, *name, *description, *author;
    uint32_t verMaj, verMin, verPat;
} FlsExtMeta;

#ifdef __cplusplus
} /* extern "C" */
#endif
