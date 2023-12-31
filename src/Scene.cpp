#include "loo/Scene.hpp"

#include <glad/glad.h>
#include <glog/logging.h>

#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <loo/glError.hpp>
#include "glm/ext/matrix_transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include "loo/Animation.hpp"

namespace std {
size_t hash<loo::Vertex>::operator()(loo::Vertex const& v) const {
    return ((hash<glm::vec3>()(v.position) ^
             (hash<glm::vec3>()(v.normal) << 1)) >>
            1) ^
           (hash<glm::vec2>()(v.texCoord) << 1);
}
}  // namespace std
namespace loo {

using namespace std;
using namespace glm;
namespace fs = std::filesystem;

void Scene::scale(glm::vec3 ratio) {
    m_scale = ratio;
}

void Scene::translate(glm::vec3 pos) {
    m_translate = pos;
}

glm::mat4 Scene::getModelMatrix() const {
    return glm::translate(glm::identity<glm::mat4>(), m_translate) *
           glm::toMat4(rotation) *
           glm::scale(glm::identity<glm::mat4>(), m_scale);
}
glm::mat4 Scene::getPreviousModelMatrix() const {
    return glm::translate(glm::identity<glm::mat4>(), m_translatePrev) *
           glm::toMat4(rotationPrev) *
           glm::scale(glm::identity<glm::mat4>(), m_scalePrev);
}

size_t Scene::countMesh() const {
    return m_meshes.size();
}

size_t Scene::countVertex() const {
    size_t cnt = 0;
    for (const auto& mesh : m_meshes) {
        cnt += mesh->countVertex();
    }
    return cnt;
}
size_t Scene::countTriangle() const {
    size_t cnt = 0;
    for (const auto& mesh : m_meshes) {
        cnt += mesh->countTriangles();
    }
    return cnt;
}

void Scene::addMeshes(vector<shared_ptr<Mesh>>&& meshes) {
    m_meshes.insert(m_meshes.end(), meshes.begin(), meshes.end());
    for (const auto& mesh : meshes) {
        aabb.merge(mesh->aabb.transform(mesh->objectMatrix));
    }
}

AABB Scene::computeAABBWorldSpace() {
    return aabb.transform(getModelMatrix());
}

// prepare the scene, move the mesh data into opengl side
void Scene::prepare() const {
    for (const auto& mesh : m_meshes) {
        mesh->prepare();
    }
}
Scene::Scene() = default;

Scene createSceneFromFile(const std::string& filename) {
    using namespace Assimp;
    Importer importer;
    const auto aiScene = importer.ReadFile(
        filename,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
            aiProcess_GenNormals | aiProcess_CalcTangentSpace |
            aiProcess_GenBoundingBoxes | aiProcess_LimitBoneWeights |
            aiProcess_ImproveCacheLocality | aiProcess_OptimizeMeshes |
            aiProcess_OptimizeGraph | aiProcess_SplitLargeMeshes |
            aiProcess_RemoveRedundantMaterials);
    string modelName = aiScene->mName.C_Str();
    fs::path modelPath(filename);
    fs::path modelDir = modelPath.parent_path();
    if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !aiScene->mRootNode) {
        LOG(FATAL) << "Assimp: " << importer.GetErrorString() << endl;
    }
    Scene scene;
    auto meshes = createMeshesFromAssimp(aiScene, scene.boneMap,
                                         scene.boneMatrices, modelDir.string());
    scene.addMeshes(std::move(meshes));
    if (scene.modelName.empty()) {
        scene.modelName = modelPath.stem().string();
    }
    scene.prepare();

    if (aiScene->HasAnimations()) {
        scene.animation = createAnimationFromAssimp(*aiScene, scene.boneMap,
                                                    scene.boneMatrices);
    }

    return std::move(scene);
}

glm::mat4 getLightSpaceTransform(glm::vec3 lightPosition) {
    vec3 target(0, 0, 0);
    vec3 dir = normalize(target - lightPosition);
    vec3 up{0.0f, 1.0f, 0.0f};
    if (dir.x == 0.0f && dir.z == 0.0f)
        up = glm::vec3(1.0f, 0.0f, 0.0f);
    return ortho(-1.5f, 1.5f, -1.5f, 1.5f, -1.5f, 1.5f) *
           lookAt(vec3(0.0f), dir, up);
}

}  // namespace loo
