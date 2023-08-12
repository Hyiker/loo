#include "loo/Bone.hpp"
#include <glog/logging.h>
#include <glm/gtx/quaternion.hpp>
#include <utility>
#include "glm/gtx/compatibility.hpp"
#include "loo/utils.hpp"
namespace loo {

int BONES_MAX_INFLUENCE = 4;
int BONES_MAX_COUNT = 200;

Bone::Bone(const std::string& name, int id, std::vector<KeyPosition> positions,
           std::vector<KeyRotation> rotations, std::vector<KeyScale> scales)
    : name(name),
      id(id),
      localTransform(1.0f),
      m_positions(std::move(positions)),
      m_rotations(std::move(rotations)),
      m_scales(std::move(scales)) {}

int Bone::getPositionIndex(float animationTime) {
    for (int index = 0; index < m_positions.size() - 1; ++index) {
        if (animationTime < m_positions[index + 1].timeStamp)
            return index;
    }
    LOG(WARNING) << "animation time not found in position keys";
    return -1;
}

int Bone::getRotationIndex(float animationTime) {
    for (int index = 0; index < m_rotations.size() - 1; ++index) {
        if (animationTime < m_rotations[index + 1].timeStamp)
            return index;
    }
    LOG(WARNING) << "animation time not found in rotation keys";
    return -1;
}

int Bone::getScaleIndex(float animationTime) {
    for (int index = 0; index < m_scales.size() - 1; ++index) {
        if (animationTime < m_scales[index + 1].timeStamp)
            return index;
    }
    LOG(WARNING) << "animation time not found in scaling keys";
    return -1;
}

float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp,
                           float animationTime) {
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 Bone::interpolatePosition(float animationTime) {
    if (1 == m_positions.size())
        return glm::translate(glm::mat4(1.0f), m_positions[0].position);

    int p0Index = getPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        getScaleFactor(m_positions[p0Index].timeStamp,
                       m_positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition =
        glm::lerp(m_positions[p0Index].position, m_positions[p1Index].position,
                  scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(float animationTime) {
    if (1 == m_rotations.size()) {
        auto rotation = glm::normalize(m_rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = getRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        getScaleFactor(m_rotations[p0Index].timeStamp,
                       m_rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation =
        glm::slerp(m_rotations[p0Index].orientation,
                   m_rotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::interpolateScaling(float animationTime) {
    if (1 == m_scales.size())
        return glm::scale(glm::mat4(1.0f), m_scales[0].scale);

    int p0Index = getScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor =
        getScaleFactor(m_scales[p0Index].timeStamp, m_scales[p1Index].timeStamp,
                       animationTime);
    glm::vec3 finalScale = glm::lerp(m_scales[p0Index].scale,
                                     m_scales[p1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}
Bone createBoneFromAssimp(const std::string& name, int id,
                          const aiNodeAnim* channel) {
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;
    for (int positionIndex = 0; positionIndex < channel->mNumPositionKeys;
         ++positionIndex) {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position = convertVec3AssimpToGLM(aiPosition);
        data.timeStamp = timeStamp;
        positions.push_back(data);
    }

    for (int rotationIndex = 0; rotationIndex < channel->mNumRotationKeys;
         ++rotationIndex) {
        aiQuaternion aiOrientation =
            channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.orientation = convertQuatAssimpToGLM(aiOrientation);
        data.timeStamp = timeStamp;
        rotations.push_back(data);
    }

    for (int keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex) {
        aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        float timeStamp = channel->mScalingKeys[keyIndex].mTime;
        KeyScale data;
        // data.scale = convertVec3AssimpToGLM(scale);
        data.scale = glm::vec3(1.0);
        data.timeStamp = timeStamp;
        scales.push_back(data);
    }
    return Bone(name, id, positions, rotations, scales);
}
}  // namespace loo