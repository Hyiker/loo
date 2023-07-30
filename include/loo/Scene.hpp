#ifndef LOO_INCLUDE_LOO_SCENE_HPP
#define LOO_INCLUDE_LOO_SCENE_HPP
#include <glad/glad.h>

#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

#include "AABB.hpp"
#include "Mesh.hpp"
#include "predefs.hpp"

namespace loo {
class LOO_EXPORT Scene {
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    glm::vec3 m_scale{1.0f, 1.0f, 1.0f}, m_translate{0.0f, 0.0f, 0.0f};

   public:
    void scale(glm::vec3 ratio);
    void translate(glm::vec3 pos);
    void prepare() const;
    glm::mat4 getModelMatrix() const;
    auto& getMeshes() const { return m_meshes; }
    auto getMeshes() { return m_meshes; }
    void clear() {
        m_meshes.clear();
        boneMap.clear();
        boneMatrices.clear();
        aabb = AABB();
    }

    // +++++ debug use +++++
    size_t countMesh() const;
    size_t countTriangle() const;
    size_t countVertex() const;
    // +++++ debug use +++++

    void addMeshes(std::vector<std::shared_ptr<Mesh>>&& meshes);
    AABB computeAABBWorldSpace();

    Scene();
    ~Scene() = default;
    std::string modelName{};
    AABB aabb;
    // map bone name to its index in the vector
    std::map<std::string, int> boneMap;
    std::vector<glm::mat4> boneMatrices;
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
};

LOO_EXPORT Scene createSceneFromFile(const std::string& filename);

LOO_EXPORT glm::mat4 getLightSpaceTransform(glm::vec3 lightPosition);
}  // namespace loo

#endif /* LOO_INCLUDE_LOO_SCENE_HPP */
