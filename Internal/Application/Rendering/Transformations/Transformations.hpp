//
// Created by wpsimon09 on 26/10/24.
//

#ifndef TRANSFROMATIONS_HPP
#define TRANSFROMATIONS_HPP

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace ApplicationCore
{

class Transformations {
private:
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;

    glm::mat4 m_modelMatrix;

    bool m_isDirty = true;

    glm::mat4 ComputeLocalModelMatrix();
public:
    Transformations();
    Transformations(glm::vec3 position, glm::vec3 scale, glm::vec3 rotations);

    void ComputeModelMatrix();
    void ComputeModelMatrix(glm::mat4 &parentGlobalMatrix);


    glm::vec3 &GetPosition() ;
    void SetPosition(const glm::vec3 &position);
    void SetPosition(float x, float y, float z);

    glm::vec3 &GetRotations();
    void SetRotations(const glm::vec3 &rotations);
    void SetRotations(float x, float y, float z);

    glm::vec3 &GetScale();
    void SetScale(const glm::vec3 &scale);
    void SetScale(float scalar);
    void SetScale(float x, float y, float z);

    bool HasChanged() {return m_isDirty;}

    glm::mat4 &GetModelMatrix() {return m_modelMatrix;}

    void SetModelMatrix(glm::mat4 modelMatrix) {m_modelMatrix = modelMatrix;}

    virtual ~Transformations()=default;
};

}

#endif //TRANSFROMATIONS_HPP
