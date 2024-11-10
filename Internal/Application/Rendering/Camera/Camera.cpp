//
// Created by wpsimon09 on 26/10/24.
//

#include "Camera.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "GLFW/glfw3.h"

    ApplicationCore::Camera::Camera(glm::vec3 center, glm::vec3 up, float radius, float minRadius,
                   float azimuthAngle, float polarAngle) {
        m_center = center;
        m_worldUp = up;

        float width = 800, height = 600;
        m_projection = glm::perspective(glm::radians(65.0f), width / height, 0.1f, 700.0f);
        m_farPlane = 700.0f;;
        m_nearPlane = 0.1f;

        m_radius = radius;
        m_minRadius = minRadius;
        m_azimuthAngle = azimuthAngle;
        m_polarAngle = polarAngle;
        m_position = getEye();
    }

    void ApplicationCore::Camera::RotateAzimutn(float radians) {
        m_azimuthAngle += radians;
        const auto fullCircle = 2.0f * glm::pi<float>();
        m_azimuthAngle = fmodf(m_azimuthAngle, fullCircle);
        if (m_azimuthAngle < 0.0f) {
            m_azimuthAngle = m_azimuthAngle + fullCircle;
        }
        m_position = getEye();
    }

    void ApplicationCore::Camera::RotatePolar(float radains) {
        m_polarAngle += radains;

        const auto polarCap = glm::pi<float>() / 2 - 0.001f;

        if (m_polarAngle > polarCap) {
            m_polarAngle = polarCap;
        }

        if (m_polarAngle < -polarCap) {
            m_polarAngle = -polarCap;
        }
        m_position = getEye();
    }

    void ApplicationCore::Camera::Zoom(float by) {
        m_radius -= by;

        if (m_radius < m_minRadius) {
            m_radius = m_minRadius;
        }
        m_position = getEye();
    }

    void ApplicationCore::Camera::MoveHorizontal(float distance) {
        if(distance > 0.f) {
            const auto pos = getEye();
            const glm::vec3 viewVector = glm::normalize(pos - m_center);
            const glm::vec3 strafeVector = glm::normalize(glm::cross(viewVector, m_worldUp));
            m_center += strafeVector + distance;
            m_position = getEye();
        }
    }

    void ApplicationCore::Camera::MoveVertical(float distance) {
        if(distance >0.0f) {
            m_center += m_worldUp + distance;
            m_position = getEye();
        }
    }

    void ApplicationCore::Camera::ProcessResize(int newWidht, int newHeight) {
        m_projection = glm::perspective(glm::radians(65.0f), (float)newWidht / (float)newHeight, 0.1f, 470.0f);
        m_farPlane = GetFarPlane();
        m_nearPlane = 0.1f;
        m_position = getEye();
    }


    void ApplicationCore::Camera::Update(CameraUpdateInfo &cameraUpdateInfo)  {
        Utils::Logger::LogInfo("Updating camera");
        cameraUpdateInfo.Print();

        RotateAzimutn(cameraUpdateInfo.RotateAzimuthValue);
        RotatePolar(cameraUpdateInfo.RotatePolarValue);
        Zoom(cameraUpdateInfo.ZoomValue);
        if(cameraUpdateInfo.NewHeight > 0.0f || cameraUpdateInfo.NewWidth > 0.0f) {
            ProcessResize(cameraUpdateInfo.NewWidth, cameraUpdateInfo.NewHeight);
        }
        MoveHorizontal(cameraUpdateInfo.MoveX);
        MoveVertical(cameraUpdateInfo.MoveY);
        cameraUpdateInfo.Reset();

    }

    glm::vec3 ApplicationCore::Camera::getEye() {
        const auto sineAzimuth = sin(m_azimuthAngle);
        const auto cosineAzimuzh = cos(m_azimuthAngle);
        const auto sinePolar = sin(m_polarAngle);
        const auto cosinePolar = cos(m_polarAngle);

        const auto x = m_center.x + m_radius * cosinePolar * cosineAzimuzh;
        const auto y = m_center.y + m_radius * sinePolar;
        const auto z = m_center.z + m_radius * cosinePolar * sineAzimuth;

        return {x, y, z};
    }

// ApplicationCore
