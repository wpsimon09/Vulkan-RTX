//
// Created by wpsimon09 on 28/01/25.
//
#include "MathUtils.hpp"
#include <glm/gtc/quaternion.hpp>


glm::vec3 MathUtils::QuaternionToEuler(fastgltf::math::fquat quat)
{
    glm::quat q = glm::quat((float)quat.w(), (float)quat.x(), (float)quat.y(), (float)quat.z());
    glm::vec3 euler =  glm::eulerAngles(q);
    return {glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z)};

}

