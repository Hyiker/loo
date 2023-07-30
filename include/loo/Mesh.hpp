#ifndef LOO_INCLUDE_LOO_MESH_HPP
#define LOO_INCLUDE_LOO_MESH_HPP
#include <memory>
#include <utility>
#include <vector>

#include "AABB.hpp"
#include "Bone.hpp"
#include "Material.hpp"
#include "Shader.hpp"
#include "predefs.hpp"

namespace loo {
struct LOO_EXPORT Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::ivec4 boneIds{-1, -1, -1, -1};
    glm::vec4 boneWeights{0.0f, 0.0f, 0.0f, 0.0f};

    bool operator==(const Vertex& v) const;
    // Re-orthogonalization tangents, modifies tangent and bitangent
    void orthogonalizeTangent();
};

struct LOO_EXPORT Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::shared_ptr<Material> material;
    std::string name;
    glm::mat4 objectMatrix;
    AABB aabb;

    Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indicies,
         std::shared_ptr<Material> material, std::string name,
         const glm::mat4& transform, AABB aabb)
        : vertices(vertices),
          indices(indicies),
          material(material),
          name(std::move(name)),
          objectMatrix(transform),
          aabb(aabb) {}

    bool needAlphaBlend() const {
        return material ? material->needAlphaBlend() : false;
    }
    bool isDoubleSided() const {
        return material ? material->isDoubleSided() : false;
    }

    GLuint vao, vbo, ebo;
    void prepare();
    size_t countVertex() const;
    size_t countTriangles() const;
};
class Animator;
LOO_EXPORT std::vector<std::shared_ptr<Mesh>> createMeshesFromFile(
    std::map<std::string, int>& boneIndexMap,
    std::vector<glm::mat4>& boneOffsetMatrices, Animator* animator,
    const std::string& filename, std::string& modelName);

}  // namespace loo

namespace std {
template <>
struct LOO_EXPORT hash<loo::Vertex> {
    size_t operator()(loo::Vertex const& v) const;
};
}  // namespace std
#endif /* LOO_INCLUDE_LOO_MESH_HPP */
