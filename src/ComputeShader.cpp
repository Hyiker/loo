#include "loo/ComputeShader.hpp"
namespace loo {
ComputeShader::ComputeShader(Shader shader)
    : ShaderProgram{std::move(shader)} {}
ComputeShader::ComputeShader(ComputeShader&& other) : ShaderProgram() {
    m_handle = other.m_handle;
    other.m_handle = GL_INVALID_INDEX;
}

ComputeShader::~ComputeShader() {
    if (m_handle != GL_INVALID_INDEX)
        glDeleteProgram(m_handle);
}
void ComputeShader::dispatch(GLuint numGroupsX, GLuint numGroupsY,
                             GLuint numGroupsZ) const {
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

}  // namespace loo