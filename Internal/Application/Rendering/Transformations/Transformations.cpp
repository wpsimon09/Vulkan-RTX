//
// Created by wpsimon09 on 26/10/24.
//

#include "Transformations.hpp"

#include <glm/gtc/quaternion.hpp>

ApplicationCore::Transformations::Transformations()
{
  m_position     = glm::vec3(0.0f, 0.0f, 0.0f);
  m_rotation     = glm::vec3(0.0f, 0.0f, 0.0f);
  m_scale        = glm::vec3(1.0f, 1.0f, 1.0f);
  m_modelMatrix  = glm::mat4(1.0f);
  m_rotationQuat = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
  m_isDirty      = false;
}

ApplicationCore::Transformations::Transformations(glm::vec3 position, glm::vec3 scale, glm::vec3 rotations)
    : m_position(position)
    , m_rotation(rotations)
    , m_scale(scale)
{
  m_modelMatrix = ComputeLocalModelMatrix();
  m_isDirty     = false;
}

ApplicationCore::Transformations::Transformations(glm::vec3 position, glm::vec3 scale, glm::quat rotations)
    : m_position(position)
    , m_rotationQuat(rotations)
    , m_scale(scale)
{
  m_isDirty  = false;
  m_rotation = glm::eulerAngles(m_rotationQuat);
}

glm::mat4 ApplicationCore::Transformations::ComputeLocalModelMatrix()
{
  //make rotation matrix
  m_rotationQuat   = glm::quat(glm::radians(m_rotation));
  m_rotationMatrix = glm::mat4_cast(m_rotationQuat);

  //compute model matrix from rotation, position and scle
  return glm::translate(glm::mat4(1.f), m_position) * m_rotationMatrix * glm::scale(glm::mat4(1.0f), m_scale);
}

void ApplicationCore::Transformations::ComputeModelMatrix()
{
  if(m_isDirty)
  {
    m_modelMatrix   = ComputeLocalModelMatrix();
    m_worldPosition = m_modelMatrix * glm::vec4(m_position, 1.0f);
    m_isDirty       = false;
  }
}


void ApplicationCore::Transformations::ComputeModelMatrix(glm::mat4& parentGlobalMatrix)
{
  m_modelMatrix   = parentGlobalMatrix * ComputeLocalModelMatrix();
  m_worldPosition = m_modelMatrix * glm::vec4(m_position, 1.0f);
  m_isDirty       = false;
}

glm::vec3& ApplicationCore::Transformations::GetPosition()
{
  return m_position;
}

void ApplicationCore::Transformations::SetPosition(const glm::vec3& position)
{
  m_position = position;
  m_isDirty  = true;
}

void ApplicationCore::Transformations::SetPosition(float x, float y, float z)
{
  m_position = glm::vec3(x, y, z);
  m_isDirty  = true;
}

glm::vec3& ApplicationCore::Transformations::GetRotations()
{
  return m_rotation;
}

glm::quat& ApplicationCore::Transformations::GetRotationsQuat()
{
  return m_rotationQuat;
}

void ApplicationCore::Transformations::SetRotations(const glm::vec3& rotations)
{
  m_rotation           = rotations;
  m_rotationQuat       = glm::quat(glm::radians(rotations));
  m_isDirty            = true;
  m_hasRotationChanged = true;
}

void ApplicationCore::Transformations::SetRotations(float x, float y, float z)
{
  m_rotation = glm::vec3(x, y, z);

  m_rotationQuat = glm::quat(glm::radians(glm::vec3(x, y, z)));

  m_isDirty            = true;
  m_hasRotationChanged = true;
}

void ApplicationCore::Transformations::SetRotation(const glm::quat& rotation)
{
  m_rotationQuat = rotation;
}

glm::vec3& ApplicationCore::Transformations::GetScale()
{
  return m_scale;
}

void ApplicationCore::Transformations::SetScale(const glm::vec3& scale)
{
  m_scale   = scale;
  m_isDirty = true;
}

void ApplicationCore::Transformations::SetScale(float scalar)
{
  m_scale.x = scalar;
  m_scale.y = scalar;
  m_scale.z = scalar;
}

void ApplicationCore::Transformations::SetScale(float x, float y, float z)
{
  m_scale   = glm::vec3(x, y, z);
  m_isDirty = true;
}
