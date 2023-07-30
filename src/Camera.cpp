#include "loo/Camera.hpp"

#include <algorithm>
#include <iostream>

#include <glm/ext/matrix_transform.hpp>
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace loo {

using namespace std;

glm::mat4 Camera::getViewMatrix() const {
    glm::mat4 view;
    getViewMatrix(view);
    return view;
}

void Camera::getViewMatrix(glm::mat4& view) const {
    glm::vec3 front = getDirection();
    view = glm::lookAt(position, position + front, worldUp);
}

void Camera::moveCamera(CameraMovement direction, float deltaTime) {
    float velocity = speed * deltaTime;
    glm::vec3 front = getDirection();
    glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
    if (direction == CameraMovement::FORWARD)
        position += front * velocity;
    if (direction == CameraMovement::BACKWARD)
        position -= front * velocity;
    if (direction == CameraMovement::LEFT)
        position -= right * velocity;
    if (direction == CameraMovement::RIGHT)
        position += right * velocity;
}

void PerspectiveCamera::zoomCamera(float value) {
    m_fov -= value * 0.05;
    m_fov = std::clamp(m_fov, 0.01f, float(glm::radians(160.f)));
}
static glm::mat4 perspectiveZInfty(float fovy, float aspect, float zNear) {
    float f = 1.0f / tan(fovy / 2.0f);
    return glm::mat4(f / aspect, 0.0f, 0.0f, 0.0f, 0.0f, f, 0.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, -1.0f, 0.0f, 0.0f, zNear, 0.0f);
}

void PerspectiveCamera::getProjectionMatrix(glm::mat4& projection,
                                            bool reverseZ01) const {
    if (reverseZ01) {
        projection = perspectiveZInfty(m_fov, m_aspect, m_znear);
    } else
        projection = glm::perspective(m_fov, m_aspect, m_znear, m_zfar);
}

glm::mat4 PerspectiveCamera::getProjectionMatrix(bool reverseZ01) const {
    glm::mat4 projection;
    getProjectionMatrix(projection, reverseZ01);
    return projection;
}

void FPSCamera::rotateCamera(float xoffset, float yoffset) {
    xoffset *= -sensitivity;
    yoffset *= sensitivity;

    m_yaw += xoffset;
    m_yaw = m_yaw > 360.0 ? m_yaw - 360.0 : m_yaw;
    m_yaw = m_yaw < -360.0 ? m_yaw + 360.0 : m_yaw;
    m_pitch += yoffset;

    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    // construct quaternion from pitch and yaw
    orientation =
        glm::quat(glm::vec3(glm::radians(m_pitch), glm::radians(m_yaw), 0.0f));
}

void ArcBallCamera::orbitCamera(float xoffset, float yoffset) {
    glm::vec3 camFocus = position - m_center;
    glm::vec3 right = getRight(getDirection());
    constexpr float THETA_MAX = glm::radians(178.0f),
                    THETA_MIN = glm::radians(2.0f);
    glm::vec3 direction = glm::normalize(camFocus);
    float theta = glm::acos(glm::dot(direction, worldUp));
    float deltaTheta = yoffset * sensitivity * 1e-2f;
    // clamp deltaTheta
    deltaTheta = std::clamp(deltaTheta, THETA_MIN - theta, THETA_MAX - theta);

    // deltaPitch = std::clamp(deltaPitch, PITCH_MIN_RAD - currentPitch,
    //                         PITCH_MAX_RAD - currentPitch);
    glm::quat rotPitch = glm::angleAxis(deltaTheta, right);
    glm::quat rotYaw = glm::angleAxis(-xoffset * sensitivity * 1e-2f, worldUp);
    camFocus = rotPitch * rotYaw * camFocus;
    position = m_center + camFocus;
    orientation = glm::quatLookAt(glm::normalize(-camFocus), worldUp);
}
void ArcBallCamera::panCamera(float xoffset, float yoffset) {
    // keep rotation, move camera position and center
    glm::vec3 right = getRight(getDirection());
    glm::vec3 up = glm::normalize(glm::cross(right, getDirection()));
    glm::vec3 delta = -xoffset * sensitivity * right +
                      -yoffset * sensitivity * up;  // delta in world space
    position += delta;
    m_center += delta;
}

void ArcBallCamera::orbitCameraAroundWorldUp(float phiInDeg) {
    glm::vec3 camFocus = position - m_center;
    glm::vec3 right = getRight(getDirection());
    glm::vec3 direction = glm::normalize(camFocus);
    glm::quat rotYaw = glm::angleAxis(glm::radians(phiInDeg), worldUp);
    camFocus = rotYaw * camFocus;
    position = m_center + camFocus;
    orientation = glm::quatLookAt(glm::normalize(-camFocus), worldUp);
}

}  // namespace loo