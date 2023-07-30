#include "loo/Material.hpp"

#include <glog/logging.h>

#include <memory>
#include <string>

#include <assimp/GltfMaterial.h>
#include <assimp/material.h>
#include <assimp/types.h>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include <loo/Shader.hpp>

namespace loo {

using namespace std;
using namespace glm;
using namespace loo;
namespace fs = std::filesystem;

static unordered_map<string, shared_ptr<Texture2D>> uniqueTexture;
static inline glm::vec3 aiColor3D2Glm(const aiColor3D& aColor) {
    return {aColor.r, aColor.g, aColor.b};
}
static inline glm::vec4 aiColor4D2Glm(const aiColor4D& aColor) {
    return {aColor.r, aColor.g, aColor.b, aColor.a};
}

static shared_ptr<Texture2D> createMaterialTextures(
    const aiMaterial* mat, aiTextureType type, fs::path objParent,
    unsigned int options = TEXTURE_OPTION_MIPMAP |
                           TEXTURE_OPTION_CONVERT_TO_LINEAR) {
    vector<Texture2D> textures;
    if (mat->GetTextureCount(type)) {
        // TODO: support multilayer texture
        aiString str;
        aiTextureMapMode mapMode;
        mat->GetTexture(type, 0, &str, nullptr, nullptr, nullptr, nullptr,
                        &mapMode);
        auto texture = createTexture2DFromFile(
            uniqueTexture, (objParent / str.C_Str()).string(), options);
        texture->setWrapFilter(
            mapMode == aiTextureMapMode_Wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        return texture;
    } else {
        return nullptr;
    }
}

static BlinnPhongWorkFlow createBlinnPhongWorkFlowFromAssimp(
    const aiMaterial* aMaterial, fs::path objParent) {
    aiColor3D color(0, 0, 0);
    aMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
    glm::vec3 ambient = aiColor3D2Glm(color);
    aMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    glm::vec3 diffuse = aiColor3D2Glm(color);
    aMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
    glm::vec3 specular = aiColor3D2Glm(color);
    color = aiColor3D(0, 0, 0);
    // using emissive to store transparent color
    // a hack for wavefront obj
    aMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
    glm::vec3 transparent = aiColor3D2Glm(color);
    float shininess, _ior;
    aMaterial->Get(AI_MATKEY_SHININESS, shininess);
    aMaterial->Get(AI_MATKEY_REFRACTI, _ior);
    return BlinnPhongWorkFlow(ambient, diffuse, specular, transparent, _ior,
                              shininess);
}

static MetallicRoughnessWorkFlow createMetallicRoughnessWorkFlowFromAssimp(
    const aiMaterial* aMaterial, fs::path objParent) {
    aiColor4D color4(0, 0, 0, 0);
    aMaterial->Get(AI_MATKEY_BASE_COLOR, color4);
    glm::vec4 baseColor = aiColor4D2Glm(color4);

    float metallic, roughness;
    aMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
    aMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

    //  baseColor textures are sRGB
    auto baseColorTex = createMaterialTextures(
        aMaterial, aiTextureType_BASE_COLOR, objParent,
        TEXTURE_OPTION_MIPMAP | TEXTURE_OPTION_CONVERT_TO_LINEAR);
    auto occlusionTex =
        createMaterialTextures(aMaterial, aiTextureType_AMBIENT_OCCLUSION,
                               objParent, TEXTURE_OPTION_MIPMAP);
    auto metallicTex = createMaterialTextures(
        aMaterial, aiTextureType_METALNESS, objParent, TEXTURE_OPTION_MIPMAP);
    auto roughnessTex =
        createMaterialTextures(aMaterial, aiTextureType_DIFFUSE_ROUGHNESS,
                               objParent, TEXTURE_OPTION_MIPMAP);

    auto workflow = MetallicRoughnessWorkFlow(baseColor, metallic, roughness);
    workflow.baseColorTex = baseColorTex;
    workflow.occlusionTex = occlusionTex;
    workflow.metallicTex = metallicTex;
    workflow.roughnessTex = roughnessTex;
    return workflow;
}

static void readGLTFMaterial(BaseMaterial& baseMaterial,
                             const aiMaterial* aMaterial) {
    aiString alphaMode;
    aMaterial->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    baseMaterial.flags |=
        !strcmp(alphaMode.C_Str(), "BLEND") ? LOO_MATERIAL_FLAG_ALPHA_BLEND : 0;
}

std::shared_ptr<BaseMaterial> createBaseMaterialFromAssimp(
    const aiMaterial* aMaterial, fs::path objParent) {
    auto blinnPhong = createBlinnPhongWorkFlowFromAssimp(aMaterial, objParent);
    auto metallicRoughness =
        createMetallicRoughnessWorkFlowFromAssimp(aMaterial, objParent);
    auto material = make_shared<BaseMaterial>(blinnPhong, metallicRoughness);

    // read common textures
    material->ambientTex =
        createMaterialTextures(aMaterial, aiTextureType_AMBIENT, objParent);

    material->diffuseTex =
        createMaterialTextures(aMaterial, aiTextureType_DIFFUSE, objParent);

    material->specularTex =
        createMaterialTextures(aMaterial, aiTextureType_SPECULAR, objParent);

    material->displacementTex = createMaterialTextures(
        aMaterial, aiTextureType_DISPLACEMENT, objParent, 0x0);
    // material->displacementTex->setSizeFilter(GL_NEAREST, GL_NEAREST);
    // obj file saves normal map as bump maps
    // FUCK YOU, wavefront obj
    material->normalTex = createMaterialTextures(
        aMaterial, aiTextureType_NORMALS, objParent, TEXTURE_OPTION_MIPMAP);
    material->opacityTex = createMaterialTextures(
        aMaterial, aiTextureType_OPACITY, objParent, TEXTURE_OPTION_MIPMAP);
    material->heightTex = createMaterialTextures(
        aMaterial, aiTextureType_HEIGHT, objParent, TEXTURE_OPTION_MIPMAP);
    material->emissiveTex = createMaterialTextures(
        aMaterial, aiTextureType_EMISSIVE, objParent,
        TEXTURE_OPTION_MIPMAP | TEXTURE_OPTION_CONVERT_TO_LINEAR);

    readGLTFMaterial(*material, aMaterial);

    int doubleSided = 0;
    aMaterial->Get(AI_MATKEY_TWOSIDED, doubleSided);
    material->flags |= doubleSided ? LOO_MATERIAL_FLAG_DOUBLE_SIDED : 0;

    aiColor3D color3(0, 0, 0);
    aMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color3);
    material->emissiveFactor = aiColor3D2Glm(color3);

    return material;
}
}  // namespace loo