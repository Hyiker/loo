#ifndef LOO_INCLUDE_LOO_MATERIAL_HPP
#define LOO_INCLUDE_LOO_MATERIAL_HPP
#include "Shader.hpp"

#include <assimp/material.h>
#include <glog/logging.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <memory>

#include <assimp/types.h>
#include <loo/Texture.hpp>
#include <loo/UniformBuffer.hpp>
#include <loo/loo.hpp>
namespace loo {
class LOO_EXPORT Material {
   public:
    // setup uniforms and textures for shader program
    virtual void bind(const ShaderProgram& sp) = 0;
    [[nodiscard]] virtual bool needAlphaBlend() const { return false; }
    [[nodiscard]] virtual bool isDoubleSided() const { return false; }
};

struct BlinnPhongWorkFlow {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 transparent;

    float ior;
    float shininess;
    BlinnPhongWorkFlow() = default;
    BlinnPhongWorkFlow(glm::vec3 a, glm::vec3 d, glm::vec3 s, glm::vec3 t,
                       float i, float sh)
        : ambient(a),
          diffuse(d),
          specular(s),
          transparent(t),
          ior(i),
          shininess(sh) {}
};
struct MetallicRoughnessWorkFlow {
    glm::vec4 baseColor;
    float metallic;
    float roughness;

    MetallicRoughnessWorkFlow() = default;
    MetallicRoughnessWorkFlow(glm::vec4 bc, float m, float r)
        : baseColor(bc), metallic(m), roughness(r) {}

    std::shared_ptr<loo::Texture2D> baseColorTex{};
    std::shared_ptr<loo::Texture2D> occlusionTex{};
    std::shared_ptr<loo::Texture2D> metallicTex{};
    std::shared_ptr<loo::Texture2D> roughnessTex{};
};

constexpr unsigned int LOO_MATERIAL_FLAG_ALPHA_BLEND = 0x1,
                       LOO_MATERIAL_FLAG_DOUBLE_SIDED = 0x2;

struct BaseMaterial : public Material {

    void bind(const ShaderProgram& sp) override { NOT_IMPLEMENTED_RUNTIME(); }
    BaseMaterial(BlinnPhongWorkFlow bp, MetallicRoughnessWorkFlow mr)
        : bpWorkFlow(bp), mrWorkFlow(mr) {}
    // common textures
    std::shared_ptr<loo::Texture2D> diffuseTex{};
    std::shared_ptr<loo::Texture2D> ambientTex{};
    std::shared_ptr<loo::Texture2D> displacementTex{};
    std::shared_ptr<loo::Texture2D> normalTex{};
    std::shared_ptr<loo::Texture2D> specularTex{};
    std::shared_ptr<loo::Texture2D> opacityTex{};
    std::shared_ptr<loo::Texture2D> heightTex{};
    std::shared_ptr<loo::Texture2D> emissiveTex{};

    // common factors
    glm::vec3 emissiveFactor;
    // blinn-phong params
    BlinnPhongWorkFlow bpWorkFlow;
    // metalness-roughness params
    MetallicRoughnessWorkFlow mrWorkFlow;

    unsigned int flags = 0;
};
std::shared_ptr<loo::BaseMaterial> createBaseMaterialFromAssimp(
    const aiMaterial* aMaterial, std::filesystem::path objParent);
}  // namespace loo
#endif /* LOO_INCLUDE_LOO_MATERIAL_HPP */
