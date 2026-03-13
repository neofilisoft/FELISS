#include "ecs/World.h"
#include "renderer/RenderPipeline.h"
#include "core/Logger.h"
#include <algorithm>
#include <fstream>

namespace Feliss {

const std::vector<EntityID> World::s_noChildren;

World::World()  { FLS_INFO("World", "ECS World created"); }
World::~World() { clear(); }

EntityID World::createEntity(const std::string& name) {
    EntityID id = generateID();
    EntityRecord rec;
    rec.id   = id;
    rec.name = name.empty() ? "Entity" : name;
    // Default components
    rec.comps[std::type_index(typeid(TagComponent))] =
        std::make_unique<TagComponent>(rec.name);
    rec.comps[std::type_index(typeid(TransformComponent))] =
        std::make_unique<TransformComponent>();
    m_entities[id] = std::move(rec);
    if (onEntityCreated) onEntityCreated(id);
    return id;
}

void World::detachFromParent(EntityID id) {
    EntityID par = getParent(id);
    if (par != NULL_ENTITY) {
        auto it = m_entities.find(par);
        if (it != m_entities.end()) {
            auto& ch = it->second.children;
            ch.erase(std::remove(ch.begin(), ch.end(), id), ch.end());
        }
    }
}

void World::destroyEntity(EntityID id) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) return;

    detachFromParent(id);

    // Recursively destroy children (copy first to avoid iterator invalidation)
    std::vector<EntityID> children = it->second.children;
    for (EntityID c : children) destroyEntity(c);

    if (onEntityDestroyed) onEntityDestroyed(id);
    m_entities.erase(id);
}

bool          World::isValid(EntityID id) const { return m_entities.count(id) > 0; }
void          World::setActive(EntityID id, bool v) {
    auto it = m_entities.find(id); if (it != m_entities.end()) it->second.active = v;
}
bool          World::isActive(EntityID id) const {
    auto it = m_entities.find(id); return it != m_entities.end() && it->second.active;
}
void          World::setName(EntityID id, const std::string& n) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) return;
    it->second.name = n;
    if (auto* t = getComponent<TagComponent>(id)) t->name = n;
}
std::string   World::getName(EntityID id) const {
    auto it = m_entities.find(id); return it != m_entities.end() ? it->second.name : "";
}
void          World::setTag(EntityID id, const std::string& t) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) return;
    it->second.tag = t;
    if (auto* tc = getComponent<TagComponent>(id)) tc->tag = t;
}
std::string   World::getTag(EntityID id) const {
    auto it = m_entities.find(id); return it != m_entities.end() ? it->second.tag : "";
}

void World::setParent(EntityID child, EntityID parent) {
    if (!isValid(child)) return;
    detachFromParent(child);
    m_entities[child].parent = parent;
    if (parent != NULL_ENTITY && isValid(parent))
        m_entities[parent].children.push_back(child);
}

EntityID World::getParent(EntityID child) const {
    auto it = m_entities.find(child);
    return it != m_entities.end() ? it->second.parent : NULL_ENTITY;
}

const std::vector<EntityID>& World::getChildren(EntityID id) const {
    auto it = m_entities.find(id);
    return it != m_entities.end() ? it->second.children : s_noChildren;
}

std::vector<EntityID> World::getRoots() const {
    std::vector<EntityID> roots;
    for (auto& [id, rec] : m_entities)
        if (rec.parent == NULL_ENTITY) roots.push_back(id);
    return roots;
}

EntityID World::findByName(const std::string& n) const {
    for (auto& [id, rec] : m_entities) if (rec.name == n) return id;
    return NULL_ENTITY;
}

std::vector<EntityID> World::findByTag(const std::string& t) const {
    std::vector<EntityID> res;
    for (auto& [id, rec] : m_entities) if (rec.tag == t) res.push_back(id);
    return res;
}

void World::update(f32 dt) {
    // Update dirty transforms
    each<TransformComponent>([](EntityID, TransformComponent& t){
        if (t.dirty) {
            // Compute local TRS matrix
            // TODO: integrate GLM for proper matrix math
            // t.worldMatrix = glm::translate(t.position) * glm::mat4_cast(t.rotation)
            //               * glm::scale(t.scale);
            t.dirty = false;
        }
    });
}

void World::render(RenderPipeline& rp) {
    // Submit mesh renderers
    each<TransformComponent, MeshRendererComponent>(
        [&](EntityID, TransformComponent& t, MeshRendererComponent& mr) {
            if (!mr.enabled) return;
            rp.submitMesh(mr.meshID, mr.materialID, t.worldMatrix,
                          mr.castShadow, mr.renderLayer);
        });

    // Submit sprites
    each<TransformComponent, SpriteComponent>(
        [&](EntityID, TransformComponent& t, SpriteComponent& s) {
            if (!s.enabled) return;
            rp.submitSprite(s.textureID, s.tint, s.size,
                            t.position, s.sortOrder);
        });

    // Submit lights
    each<TransformComponent, LightComponent>(
        [&](EntityID, TransformComponent& t, LightComponent& l) {
            if (!l.enabled) return;
            Vec3 dir = {0, -1, 0}; // TODO: derive from rotation
            rp.submitLight(static_cast<int>(l.type), l.color,
                           l.intensity, t.position, dir, l.range);
        });
}

void World::clear() {
    m_entities.clear();
    m_nextID = 1;
    FLS_INFO("World", "World cleared");
}

bool World::saveToFile(const std::string& path) const {
    std::ofstream f(path);
    if (!f.is_open()) return false;
    // Minimal JSON-style output (replace with proper serializer later)
    f << "{\n  \"entities\": [\n";
    bool first = true;
    for (auto& [id, rec] : m_entities) {
        if (!first) f << ",\n";
        first = false;
        f << "    {\"id\":" << id
          << ",\"name\":\"" << rec.name << "\""
          << ",\"active\":" << (rec.active ? "true" : "false")
          << ",\"parent\":" << rec.parent << "}";
    }
    f << "\n  ]\n}\n";
    FLS_INFOF("World", "Saved " << m_entities.size() << " entities -> " << path);
    return true;
}

bool World::loadFromFile(const std::string& /*path*/) {
    // TODO: implement proper JSON deserialization
    FLS_WARN("World", "loadFromFile: not yet implemented");
    return false;
}

} // namespace Feliss
