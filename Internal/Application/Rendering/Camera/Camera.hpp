//
// Created by wpsimon09 on 26/10/24.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Application/Structs/ApplicationStructs.hpp"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE


struct CameraUpdateInfo;
class GLFWwindow;
namespace ApplicationCore {
class Camera
{
  public:
    explicit Camera(glm::vec3 center       = glm::vec3(0.0f, 20.0f, 0.0f),
                    glm::vec3 up           = glm::vec3(0.0f, 1.0f, 0.0f),
                    float     radius       = 1.0f,
                    float     minRadius    = 1.0f,
                    float     azimuthAngle = 10.0f,
                    float     polarAngle   = 10.0f);

    void RotateAzimutn(float radians);

    void RotatePolar(float radains);

    void Zoom(float by);

    void MoveHorizontal(float distance);

    void MoveForward(float distance);

    void MoveVertical(float distance);

    void ProcessResize(int newWidht, int newHeight);

    glm::mat4& GetProjectionMatrix() { return this->m_projection; }

    glm::mat4& GetViewMatrix() { return m_view; }

    glm::mat4 GetInverseViewMatrix() const
    {
        return glm::inverse(glm::lookAt(this->m_position, m_center, this->m_worldUp));
    }

    glm::vec2 GetScreenSize() const { return m_screenSize; }

    glm::mat4 GetinverseProjectionMatrix() const { return glm::inverse(m_projection); };

    glm::vec3 GetPosition() const { return this->m_position; };

    // point should be in NDC (-1, 1)
    glm::vec3 Deproject(glm::vec2 point);

    glm::vec2 GetCameraPlaneWidthAndHeight() const;

    float& GetFarPlane() { return this->m_farPlane; }

    float& GetNearPlane() { return this->m_nearPlane; }

    float& GetSpeed() { return this->m_speed; }

    float& GetFOV() { return this->m_FOV; }

    float& GetFocalLength() { return this->m_focalLength; }

    float& GetMaxReccursion() { return this->m_maxReccursion; }

    float& GetDefocuseStrength() { return this->m_defocuseStrength; }

    void SetPosition(glm::vec3& newPosition);

    void Update(CameraUpdateInfo& cameraUpdateInfo, SceneUpdateFlags& sceneUpdateFlags);

    void Recalculate();

    ~Camera() = default;

  private:
    glm::vec3 getEye();

    float m_radius;
    float m_minRadius;
    float m_azimuthAngle;
    float m_polarAngle;
    float m_FOV = 65.0f;
    float m_aspect;
    float m_defocuseStrength = 0.2f;
    float m_speed            = 3.3f;
    float m_focalLength      = 0.35f;  // focal lenght is in CM
    float m_maxReccursion    = 10;     // max ray reccrusion from the camera

    glm::vec3 m_position;
    glm::vec3 m_center;
    glm::vec3 m_worldUp;
    glm::vec2 m_screenSize;


    glm::mat4 m_projection = glm::mat4(1.0f);
    glm::mat4 m_view       = glm::mat4(1.0f);

    float m_farPlane;
    float m_nearPlane;

    CameraUpdateInfo m_previusUpdateInfo;
    bool             m_hasChanged = true;
};

}  // namespace ApplicationCore

#endif  //CAMERA_HPP
