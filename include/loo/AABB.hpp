#ifndef LOO_INCLUDE_LOO_AABB_HPP
#define LOO_INCLUDE_LOO_AABB_HPP

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace loo {
struct AABB {
    glm::vec3 min, max;
    AABB();
    AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
    AABB(const AABB& aabb) : min(aabb.min), max(aabb.max) {}
    void merge(const AABB& aabb);
    AABB transform(const glm::mat4& mat) const;
    glm::vec3 getCenter() const;
    glm::vec3 getDiagonal() const;
    float getVolume() const;
};
};  // namespace loo

#endif /* LOO_INCLUDE_LOO_AABB_HPP */
