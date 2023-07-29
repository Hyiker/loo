#include "loo/Animation.hpp"
#include <glm/ext/matrix_transform.hpp>
#include "loo/utils.hpp"
namespace loo {
static void readMissingBones(const aiAnimation* animation,
                             std::vector<Bone>& bones,
                             std::map<std::string, int>& boneInfoMap,
                             std::vector<glm::mat4>& boneMatrices) {
    int size = animation->mNumChannels;

    //reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName] = boneMatrices.size();
            boneMatrices.push_back(glm::identity<glm::mat4>());
        }
        bones.push_back(
            createBoneFromAssimp(boneName, boneInfoMap[boneName], channel));
    }
}

static AssimpNodeData readHeirarchyData(const aiNode* src) {
    AssimpNodeData dest;

    dest.name = src->mName.data;
    dest.transformation = convertMat4AssimpToGLM(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData = readHeirarchyData(src->mChildren[i]);
        dest.children.push_back(newData);
    }
    return dest;
}

std::unique_ptr<Animation> createAnimationFromAssimp(
    const aiScene& assimpScene, std::map<std::string, int>& boneIndexMap,
    std::vector<glm::mat4>& boneOffsetMatrices) {
    assert(scene && scene->mRootNode);
    if (!assimpScene.HasAnimations())
        return nullptr;
    auto animation = assimpScene.mAnimations[0];
    float duration = animation->mDuration;
    float ticksPerSecond = animation->mTicksPerSecond;
    std::vector<Bone> bones;
    auto rootNode = readHeirarchyData(assimpScene.mRootNode);
    readMissingBones(animation, bones, boneIndexMap, boneOffsetMatrices);
    return std::make_unique<Animation>(duration, ticksPerSecond, bones,
                                       rootNode, boneIndexMap,
                                       boneOffsetMatrices);
}

}  // namespace loo