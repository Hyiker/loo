#ifndef LOO_INCLUDE_LOO_BONE_HPP
#define LOO_INCLUDE_LOO_BONE_HPP
#include <assimp/anim.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>
namespace loo {
extern int BONES_MAX_INFLUENCE;
extern int BONES_MAX_COUNT;
struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

class Bone {
   private:
    std::vector<KeyPosition> m_positions;
    std::vector<KeyRotation> m_rotations;
    std::vector<KeyScale> m_scales;

   public:
    /*reads keyframes from aiNodeAnim*/
    Bone(const std::string& name, int id, std::vector<KeyPosition> positions,
         std::vector<KeyRotation> rotations, std::vector<KeyScale> scales);

    /*interpolates  b/w positions,rotations & scaling keys based on the curren time of 
    the animation and prepares the local transformation matrix by combining all keys 
    tranformations*/
    void update(float animationTime) {
        glm::mat4 translation = interpolatePosition(animationTime);
        glm::mat4 rotation = interpolateRotation(animationTime);
        glm::mat4 scale = interpolateScaling(animationTime);
        localTransform = translation * rotation * scale;
    }

    int getPositionIndex(float animationTime);
    int getRotationIndex(float animationTime);
    int getScaleIndex(float animationTime);

    glm::mat4 localTransform;
    std::string name;
    int id;

   private:
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp,
                         float animationTime);

    glm::mat4 interpolatePosition(float animationTime);

    glm::mat4 interpolateRotation(float animationTime);

    glm::mat4 interpolateScaling(float animationTime);
};

Bone createBoneFromAssimp(const std::string& name, int id,
                          const aiNodeAnim* channel);
}  // namespace loo
#endif /* LOO_INCLUDE_LOO_BONE_HPP */
