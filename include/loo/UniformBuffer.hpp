#ifndef LOO_LOO_UNIFORM_BUFFER_HPP
#define LOO_LOO_UNIFORM_BUFFER_HPP
#include <glad/glad.h>

#include "loo.hpp"
namespace loo {

class LOO_EXPORT UniformBuffer {
   public:
    UniformBuffer(int bindPoint, size_t dataSize, void* dataPtr = nullptr);
    UniformBuffer(UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&& other);
    void updateData(int offset, size_t dataSize, void* dataPtr) const {
        glNamedBufferSubData(m_handle, offset, dataSize, dataPtr);
    }

    void updateData(void* dataPtr) const {
        glNamedBufferSubData(m_handle, 0, m_datasize, dataPtr);
    }

    template <typename T>
    T& mapBuffer() const {
        return *reinterpret_cast<T*>(glMapNamedBuffer(m_handle, GL_READ_WRITE));
    }

    void unmapBuffer() const { glUnmapNamedBuffer(m_handle); }

    template <typename T>
    void mapBufferScoped(std::function<void(T&)> func) const {
        T* ptr =
            reinterpret_cast<T*>(glMapNamedBuffer(m_handle, GL_READ_WRITE));
        func(*ptr);
        glUnmapNamedBuffer(m_handle);
    }
    auto getBindPoint() const { return m_bindpoint; }
    ~UniformBuffer() {
        if (m_handle != GL_INVALID_INDEX)
            glDeleteBuffers(1, &m_handle);
    }

   private:
    int m_bindpoint{-1};
    size_t m_datasize{0};
    GLuint m_handle{GL_INVALID_INDEX};
};
}  // namespace loo

#endif /* LOO_LOO_UNIFORM_BUFFER_HPP */
