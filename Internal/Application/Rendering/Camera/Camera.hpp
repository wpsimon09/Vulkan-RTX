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
    explicit Camera(glm::vec3 center = glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                    float radius = 300.0f, float minRadius = 1.0f, float azimuthAngle = 10.0f,
                    float polarAngle = 10.0f);

    void RotateAzimutn(float radians);

    void RotatePolar(float radains);

    void Zoom(float by);

    void MoveHorizontal(float distance);

    void MoveForward(float distance);

    void MoveVertical(float distance);

    void ProcessResize(int newWidht, int newHeight);

    glm::mat4& GetProjectionMatrix()  { return this->m_projection;}

    glm::mat4& GetViewMatrix() {return m_view;}

    glm::mat4 GetInverseViewMatrix() const {return glm::inverse(glm::lookAt(this->m_position,m_center, this->m_worldUp)); }

    glm::vec2 GetScreenSize() const {return m_screenSize;};

    glm::vec3 GetPosition() const {return this->m_position;};

    // point should be in NDC (-1, 1)
    glm::vec3 Deproject(glm::vec2 point);

    glm::vec2 GetCameraPlaneWidthAndHeight() const;

    float& GetFarPlane()  {return this->m_farPlane;}

    float& GetNearPlane()  {return this->m_nearPlane;}

    float& GetSpeed() {return this->m_speed;}

    float& GetFOV() {return this->m_FOV;}

    void Update(CameraUpdateInfo& cameraUpdateInfo);

    ~Camera() = default;

private:
    glm::vec3 getEye();

    float m_radius;
    float m_minRadius;
    float m_azimuthAngle;
    float m_polarAngle;
    float m_FOV = 65.0f;
    float m_aspect;
    float m_speed = 3.3f;

    glm::vec3 m_position;
    glm::vec3 m_center;
    glm::vec3 m_worldUp;
    glm::vec2 m_screenSize;


    glm::mat4 m_projection = glm::mat4(1.0f);
    glm::mat4 m_view = glm::mat4(1.0f);

    float m_farPlane;
    float m_nearPlane;
};

} // ApplicationCore

#endif //CAMERA_HPP
