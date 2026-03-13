#pragma once
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <optional>
#include <variant>
#include <string_view>
#include <array>
#include <type_traits>

// =====================================================================
// Feliss Engine — Core Type System
// Header-only, no dependencies outside standard library
// =====================================================================
namespace Feliss {

// ---- Primitive aliases ----
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
using byte_t = uint8_t;

// ---- IDs ----
using EntityID    = u64;
using ComponentID = u32;
using AssetID     = u64;
using ListenerID  = u64;
using LayerMask   = u32;
static constexpr EntityID   NULL_ENTITY  = 0;
static constexpr AssetID    NULL_ASSET   = 0;
static constexpr ListenerID NULL_LISTENER = 0;

// ---- Version ----
struct Version {
    u32 major = 0, minor = 1, patch = 0;
    std::string toString() const {
        return std::to_string(major) + "." +
               std::to_string(minor) + "." +
               std::to_string(patch);
    }
    bool operator==(const Version& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
};

// ---- Status ----
enum class Status : u32 {
    OK = 0, Error, NotFound, InvalidArg,
    NotSupported, AlreadyExists, OutOfMemory, Timeout,
};

template<typename T>
struct Result {
    T       value {};
    Status  status = Status::OK;
    std::string message;
    bool ok()   const { return status == Status::OK; }
    bool fail() const { return status != Status::OK; }
    static Result success(T val)
        { return { std::move(val), Status::OK, "" }; }
    static Result error(Status s, std::string_view msg)
        { return { T{}, s, std::string(msg) }; }
};
template<> struct Result<void> {
    Status status = Status::OK;
    std::string message;
    bool ok()   const { return status == Status::OK; }
    bool fail() const { return status != Status::OK; }
    static Result success()     { return { Status::OK, "" }; }
    static Result error(Status s, std::string_view msg)
        { return { s, std::string(msg) }; }
};

// ---- Smart pointers ----
template<typename T> using Ref    = std::shared_ptr<T>;
template<typename T> using WeakRef= std::weak_ptr<T>;
template<typename T> using Scope  = std::unique_ptr<T>;

template<typename T, typename... Args>
Ref<T> MakeRef(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }
template<typename T, typename... Args>
Scope<T> MakeScope(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

// ---- UUID ----
struct UUID {
    u64 hi = 0, lo = 0;
    bool operator==(const UUID& o) const { return hi==o.hi && lo==o.lo; }
    bool operator!=(const UUID& o) const { return !(*this==o); }
    bool isNull() const { return hi==0 && lo==0; }
    static UUID generate();
    std::string toString() const;
};

// ---- Math Types ----
struct Vec2 {
    f32 x = 0, y = 0;
    Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2 operator*(f32 s)         const { return {x*s, y*s}; }
    f32  length() const { return std::sqrt(x*x + y*y); }
};
struct Vec3 {
    f32 x = 0, y = 0, z = 0;
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(f32 s)         const { return {x*s, y*s, z*s}; }
    Vec3 operator-() const { return {-x,-y,-z}; }
    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    f32  dot(const Vec3& o)   const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x};
    }
    f32  length() const { return std::sqrt(x*x+y*y+z*z); }
    Vec3 normalized() const {
        f32 l = length();
        return (l>1e-7f) ? Vec3{x/l,y/l,z/l} : Vec3{0,0,0};
    }
    static Vec3 zero()    { return {0,0,0}; }
    static Vec3 one()     { return {1,1,1}; }
    static Vec3 up()      { return {0,1,0}; }
    static Vec3 forward() { return {0,0,1}; }
    static Vec3 right()   { return {1,0,0}; }
};
struct Vec4 {
    f32 x = 0, y = 0, z = 0, w = 0;
    Vec4 operator+(const Vec4& o) const { return {x+o.x,y+o.y,z+o.z,w+o.w}; }
    Vec4 operator*(f32 s)         const { return {x*s,y*s,z*s,w*s}; }
};
struct IVec2 { i32 x = 0, y = 0; };
struct IVec3 { i32 x = 0, y = 0, z = 0; };

struct Quat {
    f32 x = 0, y = 0, z = 0, w = 1;
    static Quat identity() { return {0,0,0,1}; }
    static Quat fromAxisAngle(Vec3 axis, f32 rad) {
        f32 s = std::sin(rad*0.5f);
        Vec3 a = axis.normalized();
        return {a.x*s, a.y*s, a.z*s, std::cos(rad*0.5f)};
    }
    Quat operator*(const Quat& o) const {
        return {
            w*o.x + x*o.w + y*o.z - z*o.y,
            w*o.y - x*o.z + y*o.w + z*o.x,
            w*o.z + x*o.y - y*o.x + z*o.w,
            w*o.w - x*o.x - y*o.y - z*o.z
        };
    }
};

struct Mat4 {
    f32 m[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
    static Mat4 identity() { return Mat4{}; }
    const f32* data() const { return &m[0][0]; }
};

struct Rect {
    f32 x = 0, y = 0, w = 0, h = 0;
    bool contains(f32 px, f32 py) const {
        return px>=x && px<=x+w && py>=y && py<=y+h;
    }
};

struct Color {
    f32 r = 1, g = 1, b = 1, a = 1;
    static Color white()  { return {1,1,1,1}; }
    static Color black()  { return {0,0,0,1}; }
    static Color red()    { return {1,0,0,1}; }
    static Color green()  { return {0,1,0,1}; }
    static Color blue()   { return {0,0,1,1}; }
    static Color clear()  { return {0,0,0,0}; }
    static Color yellow() { return {1,1,0,1}; }
    u32 toRGBA8() const {
        return ((u32)(r*255)<<24)|((u32)(g*255)<<16)|
               ((u32)(b*255)<<8)|(u32)(a*255);
    }
};

// ---- Transform ----
struct Transform {
    Vec3 position = {0,0,0};
    Quat rotation = Quat::identity();
    Vec3 scale    = {1,1,1};
};

// ---- Render ----
enum class RenderAPI : u32 {
    None      = 0,
    OpenGL    = 1,
    Vulkan    = 2,
    DirectX11 = 3,
    DirectX12 = 4,
    Metal     = 5,
    WebGL     = 6,  // future
};

inline const char* RenderAPIToString(RenderAPI api) {
    switch(api) {
        case RenderAPI::OpenGL:    return "OpenGL";
        case RenderAPI::Vulkan:    return "Vulkan";
        case RenderAPI::DirectX11: return "DirectX 11";
        case RenderAPI::DirectX12: return "DirectX 12";
        case RenderAPI::Metal:     return "Metal";
        case RenderAPI::WebGL:     return "WebGL";
        default:                   return "None";
    }
}

enum class PixelFormat : u32 {
    Unknown=0, RGBA8, RGB8, RGBA16F, RGB16F,
    R32F, RG32F, RGBA32F, Depth24Stencil8, Depth32F,
};
enum class BlendMode : u32  { Opaque=0, Alpha, Additive, Multiply, Screen };
enum class CullMode  : u32  { None=0, Front, Back };
enum class FillMode  : u32  { Solid=0, Wireframe };
enum class TextureWrap : u32 { Repeat=0, Clamp, Mirror };
enum class TextureFilter : u32 { Nearest=0, Linear, Trilinear, Anisotropic };

struct Viewport {
    f32 x=0, y=0, width=1280, height=720;
    f32 minDepth=0.0f, maxDepth=1.0f;
    f32 aspectRatio() const { return width / height; }
};

// ---- Engine ----
enum class EngineMode : u32 { Game=0, Editor=1, Server=2 };
enum class LogLevel   : u32 { Trace=0, Debug, Info, Warning, Error, Fatal };

// ---- Input ----
enum class KeyCode : i32 {
    Unknown=-1, Space=32,
    Apostrophe=39, Comma=44, Minus=45, Period=46, Slash=47,
    N0=48,N1,N2,N3,N4,N5,N6,N7,N8,N9,
    Semicolon=59, Equal=61,
    A=65,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,
    Escape=256, Enter=257, Tab=258, Backspace=259, Insert=260,
    Delete=261, Right=262, Left=263, Down=264, Up=265,
    F1=290,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,
    LeftShift=340,LeftControl=341,LeftAlt=342,
    RightShift=344,RightControl=345,RightAlt=346,
};
enum class MouseButton : i32 { Left=0, Right=1, Middle=2, B4=3, B5=4 };

// ---- Assets ----
enum class AssetType : u32 {
    Unknown=0, Texture, Mesh, Shader, Material,
    Audio, Script, Scene, Prefab, Font, Animation,
};
struct AssetMeta {
    AssetID     id   = NULL_ASSET;
    std::string path;
    std::string name;
    AssetType   type = AssetType::Unknown;
    UUID        uuid;
};

// ---- Extension ----
enum class ExtensionType : u32 {
    Unknown=0, Renderer, Physics, Scripting, Audio, Editor, Game,
};
struct ExtensionInfo {
    std::string   id, name, description, author;
    Version       version;
    ExtensionType type = ExtensionType::Unknown;
    std::string   libraryPath;
    bool          enabled = true;
};

} // namespace Feliss

// ---- std::hash specialisations ----
namespace std {
template<> struct hash<Feliss::UUID> {
    size_t operator()(const Feliss::UUID& u) const noexcept {
        return hash<uint64_t>()(u.hi) ^ (hash<uint64_t>()(u.lo) << 1);
    }
};
} // namespace std
