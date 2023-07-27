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
    void link() {}
};
}  // namespace loo
#endif /* LOO_INCLUDE_LOO_COMPUTE_SHADER_HPP */
