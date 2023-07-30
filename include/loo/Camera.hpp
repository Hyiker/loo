#ifndef LOO_INCLUDE_LOO_CAMERA_HPP
#define LOO_INCLUDE_LOO_CAMERA_HPP

#include <glad/glad.h>

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#include "glm/trigonometric.hpp"
#include "predefs.hpp"

namespace loo {
enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

const glm::vec3 CAMERA_ORIENTATION_BASE{0.f, 0.f, -1.f};

class LOO_EXPORT Camera {
   public:
    // Camera control parameters
    float speed{10.0f};

    Camera() {}

    Camera(glm::vec3 position, glm::vec3 center, glm::vec3 worldUp, float zNear,
           float zFar)
        : position(position),
          worldUp(glm::normalize(worldUp)),
          m_znear(zNear),
          m_zfar(zFar) {
        orientation =
            glm::quatLookAt(glm::normalize(center - position), worldUp);
    }
    glm::mat4 getViewMatrix() const;
    void getViewMatrix(glm::mat4& view) const;
    virtual glm::mat4 getProjectionMatrix(bool reverseZ01 = false) const = 0;
    virtual void getProjectionMatrix(glm::mat4& projection,
                                     bool reverseZ01 = false) const = 0;

    void moveCamera(CameraMovement direction, float deltaTime);
    virtual void zoomCamera(float value) = 0;

    glm::vec3 position{0.f, 0.f, 0.f};
    glm::vec3 getDirection() const {
        return orientation * CAMERA_ORIENTATION_BASE;
    }
    const glm::vec3 worldUp{0.f, 1.f, 0.f};

    auto getZNear() const { return m_znear; }
    auto getZFar() const { return m_zfar; }

   protected:
    float getPitch() const { return glm::pitch(orientation); }
    float getYaw() const { return glm::yaw(orientation); }
    float getRoll() const { return glm::roll(orientation); }
    glm::vec3 getRight(const glm::vec3& direction) const {
        auto d = glm::normalize(direction);
        return glm::normalize(glm::cross(d, worldUp));
    }
    glm::vec3 getLocalUp(const glm::vec3& direction) const {
        return glm::normalize(glm::cross(getRight(direction), direction));
    }

    glm::quat orientation{1.f, 0.f, 0.f, 0.f};

    // Camera parameters
    float m_znear{0.01f};
    float m_zfar{50.f};
};
class LOO_EXPORT PerspectiveCamera : public Camera {
   public:
    PerspectiveCamera() = default;
    PerspectiveCamera(glm::vec3 position, glm::vec3 center, glm::vec3 worldUp,
                      float zNear, float zFar, float fovDeg,
                      float aspect = 4.f / 3.f)
        : Camera(position, center, worldUp, zNear, zFar),
          m_fov(glm::radians(fovDeg)),
          m_aspect(aspect) {}

    void getProjectionMatrix(glm::mat4& projection,
                             bool reverseZ01 = false) const override;
    glm::mat4 getProjectionMatrix(bool reverseZ01 = false) const override;
    void zoomCamera(float value) override;
    void setAspect(float aspect) { m_aspect = aspect; }
    void setAspect(int width, int height) { m_aspect = float(width) / height; }
    auto getFov() const { return m_fov; }
    auto getAspect() const { return m_aspect; }

   protected:
    float m_fov{float(M_PI) / 3.0f};
    float m_aspect{4.f / 3.f};
};

class LOO_EXPORT FPSCamera : public PerspectiveCamera {
   public:
    FPSCamera() : PerspectiveCamera() {
        m_pitch = glm::degrees(getPitch());
        m_yaw = glm::degrees(getYaw());
    }
    FPSCamera(glm::vec3 position, glm::vec3 center, glm::vec3 worldUp,
              float zNear, float zFar, float fovDeg, float aspect = 4.f / 3.f)
        : PerspectiveCamera(position, center, worldUp, zNear, zFar, fovDeg,
                            aspect) {
        m_pitch = glm::degrees(getPitch());
        m_yaw = glm::degrees(getYaw());
    }
    explicit FPSCamera(const PerspectiveCamera& camera)
        : FPSCamera(camera.position, camera.position + camera.getDirection(),
                    camera.worldUp, camera.getZNear(), camera.getZFar(),
                    glm::degrees(camera.getFov()), camera.getAspect()) {}
    void rotateCamera(float xoffset, float yoffset);

   private:
    float sensitivity{0.2f};
    // values are stored in degrees
    float m_pitch{0.0f}, m_yaw{0.0f};
};

class LOO_EXPORT ArcBallCamera : public PerspectiveCamera {
   public:
    ArcBallCamera() : PerspectiveCamera() {
        m_center = position + getDirection();
    }
    ArcBallCamera(glm::vec3 position, glm::vec3 center, glm::vec3 worldUp,
                  float zNear, float zFar, float fovDeg,
                  float aspect = 4.f / 3.f)
        : PerspectiveCamera(position, center, worldUp, zNear, zFar, fovDeg,
                            aspect),
          m_center(center) {}
    explicit ArcBallCamera(const PerspectiveCamera& camera)
        : ArcBallCamera(camera.position,
                        camera.position + camera.getDirection(), camera.worldUp,
                        camera.getZNear(), camera.getZFar(),
                        glm::degrees(camera.getFov()), camera.getAspect()) {}
    void orbitCamera(float xoffset, float yoffset);
    void panCamera(float xoffset, float yoffset);
    void orbitCameraAroundWorldUp(float phiInDeg);
    void setCenter(glm::vec3 center) {
        m_center = center;
        orientation =
            glm::quatLookAt(glm::normalize(m_center - position), worldUp);
    }

   private:
    float sensitivity{0.2f};
    // values are stored in degrees
    glm::vec3 m_center{0.f, 0.f, 0.f};
};

}  // namespace loo

#endif /* LOO_INCLUDE_LOO_CAMERA_HPP */
