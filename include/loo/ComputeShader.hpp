#ifndef LOO_INCLUDE_LOO_COMPUTE_SHADER_HPP
#define LOO_INCLUDE_LOO_COMPUTE_SHADER_HPP
#include "Shader.hpp"
namespace loo {
class LOO_EXPORT ComputeShader : public ShaderProgram {
    GLuint m_handle;

   public:
    ComputeShader(Shader shaderList);
    ComputeShader(ComputeShader&& other);
    ~ComputeShader() override;
    void dispatch(GLuint numGroupsX, GLuint numGroupsY = 1,
                  GLuint numGroupsZ = 1) const;
    void wait(GLbitfield barriers = GL_ALL_BARRIER_BITS) const;

    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBindImageTexture.xhtml
    template <GLenum Target>
    void setTexture(int unit, const Texture<Target>& tex, GLint level,
                    GLenum access, GLenum format, bool layered = false,
                    GLint layer = 0) const {
        glBindImageTexture(unit, tex.getId(), level,
                           layered ? GL_TRUE : GL_FALSE, layer, access, format);
    }

   private:
    // disable the following functions
    GLint attribute(const std::string& name) { return -1; }
    void setAttribute(const std::string& name, GLint size, GLsizei stride,
                      GLuint offset, GLboolean normalize, GLenum type) {}
    void setAttribute(const std::string& name, GLint size, GLsizei stride,
                      GLuint offset, GLboolean normalize) {}
    void setAttribute(const std::string& name, GLint size, GLsizei stride,
                      GLuint offset, GLenum type) {}
    void setAttribute(const std::string& name, GLint size, GLsizei stride,
                      GLuint offset) {}
    void setTexture(const std::string& name, int index, int texId,
                    GLenum texType = GL_TEXTURE_2D) {}
    template <GLenum Target>
    // opengl 4.5+ texture binding interface
    void setTexture(int unit, const Texture<Target>& tex) const {}
};
}  // namespace loo
#endif /* LOO_INCLUDE_LOO_COMPUTE_SHADER_HPP */
