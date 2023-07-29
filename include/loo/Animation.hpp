#ifndef LOO_INCLUDE_LOO_ANIMATION_HPP
#define LOO_INCLUDE_LOO_ANIMATION_HPP
#include <assimp/scene.h>
#include <glm/matrix.hpp>
#include <map>
#include <string>
#include <vector>
#include "Bone.hpp"
#include "Scene.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
namespace loo {
struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation {
   public:
    Animation() = default;

    Animation(float duration, int ticksPerSecond, std::vector<Bone> bones,
              AssimpNodeData rootNode, std::map<std::string, int> boneIndexMap,
              std::vector<glm::mat4> boneMatrices)
        : duration(duration),
          ticksPerSecond(ticksPerSecond),
          bones(std::move(bones)),
          rootNode(std::move(rootNode)),
          boneIndexMap(std::move(boneIndexMap)),
          boneMatrices(std::move(boneMatrices)) {}

    ~Animation() = default;

    Bone* findBone(const std::string& name) {
        auto iter =
            std::find_if(bones.begin(), bones.end(),
                         [&](const Bone& Bone) { return Bone.name == name; });
        if (iter == bones.end())
            return nullptr;
        else
            return &(*iter);
    }

    float duration;
    int ticksPerSecond;
    std::vector<Bone> bones;
    AssimpNodeData rootNode;

    std::map<std::string, int> boneIndexMap;
    std::vector<glm::mat4> boneMatrices;
};

class Animator {
   public:
    Animator(std::unique_ptr<Animation> animation)
        : m_currentAnimation(std::move(animation)), m_currentTime(0.0f) {

        finalBoneMatrices.reserve(BONES_MAX_COUNT);

        for (int i = 0; i < BONES_MAX_COUNT; i++)
            finalBoneMatrices.push_back(glm::identity<glm::mat4>());
    }

    void updateAnimation(float dt) {
        if (m_currentAnimation) {
            m_currentTime += m_currentAnimation->ticksPerSecond * dt;
            m_currentTime = fmod(m_currentTime, m_currentAnimation->duration);
            calculateBoneTransform(&m_currentAnimation->rootNode,
                                   glm::mat4(1.0f));
        }
    }

    void resetAnimation(std::unique_ptr<Animation> animation) {
        m_currentAnimation = std::move(animation);
        m_currentTime = 0.0f;
    }

    void calculateBoneTransform(const AssimpNodeData* node,
                                glm::mat4 parentTransform) {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* bone = m_currentAnimation->findBone(nodeName);

        if (bone) {
            bone->update(m_currentTime);
            nodeTransform = bone->localTransform;
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_currentAnimation->boneIndexMap;
        if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
            int index = boneInfoMap[nodeName];
            glm::mat4 offset = m_currentAnimation->boneMatrices[index];
            finalBoneMatrices[index] = globalTransformation * offset;
            // std::cout << index << ": "
            //           << glm::to_string(finalBoneMatrices[index]) << std::endl;
        }

        for (int i = 0; i < node->childrenCount; i++)
            calculateBoneTransform(&node->children[i], globalTransformation);
    }

    bool hasAnimation() { return m_currentAnimation != nullptr; }

    std::vector<glm::mat4> finalBoneMatrices;
    std::vector<glm::mat4> boneMatrices;

   private:
    std::unique_ptr<Animation> m_currentAnimation;
    float m_currentTime;
};

std::unique_ptr<Animation> createAnimationFromAssimp(
    const aiScene& assimpScene, std::map<std::string, int>& boneIndexMap,
    std::vector<glm::mat4>& boneOffsetMatrices);
}  // namespace loo
#endif /* LOO_INCLUDE_LOO_ANIMATION_HPP */
