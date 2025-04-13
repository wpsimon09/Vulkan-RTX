//
// Created by wpsimon09 on 28/01/25.
//

#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include <glm/glm.hpp>

#include "fastgltf/math.hpp"

namespace MathUtils {
glm::vec3             QuaternionToEuler(fastgltf::math::fquat quat);
fastgltf::math::fquat EulerToQuaternion(glm::vec3& euler);

}  // namespace MathUtils

#endif  //MATHUTILS_HPP
