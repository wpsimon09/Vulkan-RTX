//
// Created by wpsimon09 on 26/10/24.
//

#include "Transformations.hpp"

ApplicationCore::Transformations::Transformations() {
    m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    m_modelMatrix = glm::mat4(1.0f);
    m_isDirty = false;
}

ApplicationCore::Transformations::Transformations(glm::vec3 position, glm::vec3 scale, glm::vec3 rotations):m_position(position), m_rotation(rotations), m_scale(scale) {
    m_modelMatrix = glm::mat4(1.0f);
    m_isDirty = false;
}

glm::mat4 ApplicationCore::Transformations::ComputeLocalModelMatrix() {
    //set rotations on X axis
    const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),
                                             glm::radians(m_rotation.x),
                                             glm::vec3(1.0f, 0.0f, 0.0f));
    //set rotations on Y axis
    const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),
                                             glm::radians(m_rotation.y),
                                             glm::vec3(0.0f, 1.0f, 0.0f));
    //set rotations on Z axis
    const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),
                                             glm::radians(m_rotation.z),
                                             glm::vec3(0.0f, 0.0f, 1.0f));
    //make rotation matrix
    const glm::mat4 rotationMatrix = transformX * transformY * transformZ;

    //compute model matrix from rotation, position and scle
    return glm::translate(glm::mat4(1.0f),m_position) *
           rotationMatrix *
           glm::scale(glm::mat4(1.0f), m_scale);
}

void ApplicationCore::Transformations::ComputeModelMatrix() {
    if(m_isDirty) {
        m_modelMatrix = ComputeLocalModelMatrix();
        m_isDirty = false;
    }
}


void ApplicationCore::Transformations::ComputeModelMatrix(glm::mat4 &parentGlobalMatrix)  {
    m_modelMatrix = parentGlobalMatrix * ComputeLocalModelMatrix();
    m_isDirty = false;
}

glm::vec3 & ApplicationCore::Transformations::GetPosition() {
    return m_position;
}

void ApplicationCore::Transformations::SetPosition(const glm::vec3 &position) {
    m_position = position;
    m_isDirty = true;
}

void ApplicationCore::Transformations::SetPosition(float x, float y, float z) {
    m_position = glm::vec3(x, y, z);
    m_isDirty = true;
}

glm::vec3 & ApplicationCore::Transformations::GetRotations() {
    return m_rotation;
}

void ApplicationCore::Transformations::SetRotations(const glm::vec3 &rotations) {
    m_rotation = rotations;
    m_isDirty = true;
}

void ApplicationCore::Transformations::SetRotations(float x, float y, float z) {
    m_rotation = glm::vec3(x, y, z);
    m_isDirty = true;
}

glm::vec3 & ApplicationCore::Transformations::GetScale() {
    return m_scale;
}

void ApplicationCore::Transformations::SetScale(const glm::vec3 &scale) {
    m_scale = scale;
    m_isDirty = true;
}

void ApplicationCore::Transformations::SetScale(float scalar)
{
    m_scale.x = scalar;
    m_scale.y = scalar;
    m_scale.z = scalar;
}

void ApplicationCore::Transformations::SetScale(float x, float y, float z) {
    m_scale = glm::vec3(x, y, z);
    m_isDirty = true;
}
