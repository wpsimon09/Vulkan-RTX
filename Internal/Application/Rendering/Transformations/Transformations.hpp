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
    void ComputeModelMatrix(const glm::mat4 &parentGlobalMatrix);

    virtual ~Transformations()=default;

    const glm::vec3 &GetPosition() const;
    void SetPosition(const glm::vec3 &position);
    void SetPosition(float x, float y, float z);

    const glm::vec3 &getRotations() const;
    void SetRotations(const glm::vec3 &rotations);
    void SetRotations(float x, float y, float z);

    const glm::vec3 &GetScale() const;
    void SetScale(const glm::vec3 &scale);
    void SetScale(float x, float y, float z);

    bool HasChanged() {return m_isDirty;}

    const glm::mat4 &GetModelMatrix() const {return this->m_modelMatrix;}

};

}

#endif //TRANSFROMATIONS_HPP
