//
// Created by wpsimon09 on 26/10/24.
//

#include "Camera.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "GLFW/glfw3.h"

ApplicationCore::Camera::Camera(glm::vec3 center, glm::vec3 up, float radius, float minRadius, float azimuthAngle, float polarAngle)
    : m_screenSize{}
{
    m_center  = center;
    m_worldUp = up;

    float width = 800, height = 600;

    m_aspect    = width / height;
    m_nearPlane = 0.1f;
    m_farPlane  = 3700.0f;
    ;
    m_FOV = 65.0f;

    m_projection = glm::perspective(glm::radians(m_FOV), m_aspect, m_nearPlane, m_farPlane);
    ;
    m_projection[1][1] *= -1;

    m_radius       = radius;
    m_minRadius    = minRadius;
    m_azimuthAngle = azimuthAngle;
    m_polarAngle   = polarAngle;
    m_position     = getEye();

    m_screenSize = {width, height};
}

void ApplicationCore::Camera::RotateAzimutn(float radians)
{
    m_azimuthAngle += radians;
    const auto fullCircle = 2.0f * glm::pi<float>();
    m_azimuthAngle        = fmodf(m_azimuthAngle, fullCircle);
    if(m_azimuthAngle < 0.0f)
    {
        m_azimuthAngle = m_azimuthAngle + fullCircle;
    }
    m_position = getEye();
}

void ApplicationCore::Camera::RotatePolar(float radains)
{
    m_polarAngle += radains;

    const auto polarCap = glm::pi<float>() / 2 - 0.001f;

    if(m_polarAngle > polarCap)
    {
        m_polarAngle = polarCap;
    }

    if(m_polarAngle < -polarCap)
    {
        m_polarAngle = -polarCap;
    }
    m_position = getEye();
}

void ApplicationCore::Camera::Zoom(float by)
{
    m_radius -= by;

    if(m_radius < m_minRadius)
    {
        m_radius = m_minRadius;
    }
    m_position = getEye();
}

void ApplicationCore::Camera::MoveHorizontal(float distance)
{
    if(distance != 0.f)
    {
        const auto      pos          = getEye();
        const glm::vec3 viewVector   = glm::normalize(pos - m_center);
        const glm::vec3 strafeVector = glm::normalize(glm::cross(viewVector, m_worldUp));
        m_center += strafeVector * (m_speed * distance);
        m_position = getEye();
    }
}

void ApplicationCore::Camera::MoveForward(float distance)
{
    if(distance != 0.0f)
    {
        const auto      pos        = getEye();
        const glm::vec3 viewVector = glm::normalize(pos - m_center);
        m_center += viewVector * (m_speed * distance);
        m_position = getEye();
    }
}

void ApplicationCore::Camera::MoveVertical(float distance)
{
    if(distance != 0.0f)
    {
        m_center += m_worldUp * (m_speed * distance);
        m_position = getEye();
    }
}

void ApplicationCore::Camera::ProcessResize(int newWidht, int newHeight)
{
    m_screenSize = {glm::max(newWidht, 1), glm::max(newHeight, 1)};
    m_aspect     = (float)m_screenSize.x / (float)m_screenSize.y;
    m_projection = glm::perspective(glm::radians(m_FOV), m_aspect, m_nearPlane, m_farPlane);
    ;
    m_projection[1][1] *= -1;
    //m_farPlane = GetFarPlane();
    m_position = getEye();
}


glm::vec3 ApplicationCore::Camera::Deproject(glm::vec2 point)
{
    float     x       = point.x;
    float     y       = point.y;
    float     z       = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    glm::vec4 ray_clip   = glm::vec4(ray_nds, 1.0f);
    auto      projection = m_projection;
    projection[1][1] *= -1;

    glm::vec4 ray_eye       = glm::inverse(projection) * ray_clip;
    ray_eye                 = glm::vec4(ray_eye.x, ray_eye.y, ray_eye.z, 0.0f);
    glm::vec4 inv_ray_world = (glm::inverse(GetViewMatrix()) * ray_eye);
    glm::vec3 ray_world     = glm::vec3(inv_ray_world.x, inv_ray_world.y, inv_ray_world.z);
    ray_world               = glm::normalize(ray_world);

    return ray_world;
}

glm::vec2 ApplicationCore::Camera::GetCameraPlaneWidthAndHeight() const
{
    // projection plane width and height

    float planeHeight = m_nearPlane * glm::tan(glm::radians(m_FOV * 0.5f)) * 2;
    float planeWidth  = planeHeight * m_aspect;

    return {planeWidth, planeHeight};
}

void ApplicationCore::Camera::SetPosition(glm::vec3& newPosition)
{
    m_position = newPosition;
}

void ApplicationCore::Camera::Update(CameraUpdateInfo& cameraUpdateInfo)
{
    //Utils::Logger::LogInfo("Updating camera");
    //cameraUpdateInfo.Print();

    RotateAzimutn(cameraUpdateInfo.RotateAzimuthValue);
    RotatePolar(cameraUpdateInfo.RotatePolarValue);
    Zoom(cameraUpdateInfo.ZoomValue);
    if(cameraUpdateInfo.NewHeight > 0.0f || cameraUpdateInfo.NewWidth > 0.0f)
    {
        ProcessResize(cameraUpdateInfo.NewWidth, cameraUpdateInfo.NewHeight);
    }
    MoveForward(cameraUpdateInfo.MoveZ);
    MoveHorizontal(cameraUpdateInfo.MoveX);
    MoveVertical(cameraUpdateInfo.MoveY);

    m_nearPlane += cameraUpdateInfo.MoveNear;


    m_view = glm::lookAt(this->m_position, m_center, this->m_worldUp);

    cameraUpdateInfo.Reset();
}

void ApplicationCore::Camera::Recalculate()
{
    m_projection = m_projection = glm::perspective(glm::radians(m_FOV), m_aspect, m_nearPlane, m_farPlane);
    ;

    m_projection[1][1] *= -1;
}

glm::vec3 ApplicationCore::Camera::getEye()
{
    const auto sineAzimuth   = sin(m_azimuthAngle);
    const auto cosineAzimuzh = cos(m_azimuthAngle);
    const auto sinePolar     = sin(m_polarAngle);
    const auto cosinePolar   = cos(m_polarAngle);

    const auto x = m_center.x + m_radius * cosinePolar * cosineAzimuzh;
    const auto y = m_center.y + m_radius * sinePolar;
    const auto z = m_center.z + m_radius * cosinePolar * sineAzimuth;


    return {x, y, z};
}

// ApplicationCore
