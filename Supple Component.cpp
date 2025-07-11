struct MaterialComponent {
    std::string shaderPath;
    std::unordered_map<std::string, float> floatParams;
    std::unordered_map<std::string, std::string> texturePaths;
};

struct MaterialInstance {
    std::shared_ptr<MaterialComponent> base;
    uint32_t gpuMaterialID;
};

class AssetManager {
public:
    static std::shared_ptr<Mesh> LoadMesh(const std::string& path);
    static std::shared_ptr<Texture> LoadTexture(const std::string& path);
    static void ClearCache();

private:
    static std::unordered_map<std::string, std::shared_ptr<Mesh>> meshPool;
    static std::unordered_map<std::string, std::shared_ptr<Texture>> texturePool;
};

enum class InputKey { W, A, S, D, Space, Escape, Ctrl, Shift };
class InputSystem {
public:
    static void Poll();
    static bool IsKeyPressed(InputKey key);
    static bool IsGamepadButtonPressed(int gamepadID, int button);
};

struct AnimationComponent {
    std::string skeletonPath;
    std::vector<std::string> clipPaths;
    int currentClip = 0;
    float playbackTime = 0.f;
};

struct Skeleton {
    struct Bone {
        std::string name;
        int parentIndex;
        float bindPose[16]; // 4x4 matrix
    };
    std::vector<Bone> bones;
};

namespace FGS {
    struct Node {
        std::string event;
        std::string action;
        std::unordered_map<std::string, std::string> params;
    };

    class Graph {
    public:
        void Connect(const std::string& from, const std::string& to);
        void Evaluate();
    };
}

class LevelEditorUI {
public:
    void Render();
};

class MaterialEditorUI {
public:
    void Render(const MaterialComponent& mat);
};
