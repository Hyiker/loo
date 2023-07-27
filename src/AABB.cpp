#include "loo/AABB.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>
#include <limits>
namespace loo {
AABB::AABB()
    : min(glm::vec3(std::numeric_limits<float>::infinity())),
      max(glm::vec3(-std::numeric_limits<float>::infinity())) {}
void AABB::merge(const AABB& aabb) {
    min = glm::min(min, aabb.min);
    max = glm::max(max, aabb.max);
}
AABB AABB::transform(const glm::mat4& mat) const {
    // loop over all 8 points of the AABB
    glm::vec3 points[8] = {
        glm::vec3(min.x, min.y, min.z), glm::vec3(min.x, min.y, max.z),
        glm::vec3(min.x, max.y, min.z), glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, min.y, min.z), glm::vec3(max.x, min.y, max.z),
        glm::vec3(max.x, max.y, min.z), glm::vec3(max.x, max.y, max.z),
    };
    AABB aabb(glm::vec3(std::numeric_limits<float>::infinity()),
              glm::vec3(-std::numeric_limits<float>::infinity()));
    for (auto& p : points) {
        auto p4 = mat * glm::vec4(p, 1.0f);
        p = glm::vec3(p4) / p4.w;
        aabb.merge(AABB(p, p));
    }
    return aabb;
}
glm::vec3 AABB::getCenter() const {
    return (min + max) / 2.0f;
}
glm::vec3 AABB::getDiagonal() const {
    return max - min;
}
float AABB::getVolume() const {
    auto d = getDiagonal();
    return d.x * d.y * d.z;
}
}  // namespace loo