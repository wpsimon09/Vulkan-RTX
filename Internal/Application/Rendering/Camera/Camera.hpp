//
// Created by wpsimon09 on 26/10/24.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraUpdateInfo;
class GLFWwindow;
namespace ApplicationCore {
class Camera {
public:
    explicit Camera(glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                    float radius = 40.0f, float minRadius = 1.0f, float azimuthAngle = -10.0f,
                    float polarAngle = 10.0f);

    void RotateAzimutn(float radians);

    void RotatePolar(float radains);

    void Zoom(float by);

    void MoveHorizontal(float distance);

    void MoveVertical(float distance);

    void ProcessResize(int newWidht, int newHeight);

    glm::mat4 GetProjectionMatrix() const {return this->m_projection;}

    glm::mat4 GetViewMatrix() const {return glm::lookAt(this->m_position,m_center, this->m_worldUp);};

    glm::vec3 GetPosition() const {return this->m_position;};

    float GetFarPlane() {return this->m_farPlane;}

    float GetNearPlane() {return this->m_nearPlane;}

    void Update(CameraUpdateInfo& cameraUpdateInfo);

    ~Camera() = default;

private:
    glm::vec3 getEye();

    float m_radius;
    float m_minRadius;
    float m_azimuthAngle;
    float m_polarAngle;

    glm::vec3 m_position;
    glm::vec3 m_center;
    glm::vec3 m_worldUp;


    glm::mat4 m_projection = glm::mat4(1.0f);

    float m_farPlane;
    float m_nearPlane;
};

} // ApplicationCore

#endif //CAMERA_HPP
