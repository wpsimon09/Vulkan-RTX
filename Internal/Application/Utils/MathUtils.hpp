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

template <class integral>
constexpr bool is_aligned(integral x, size_t a) noexcept
{
    return (x & (integral(a) - 1)) == 0;
}

template <class integral>
constexpr integral align_up(integral x, size_t a) noexcept
{

    return integral((x + (integral(a) - 1)) & ~integral(a - 1));
}

inline uint32_t alignedSize(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}


template <class integral>
constexpr integral align_down(integral x, size_t a) noexcept
{
    return integral(x & ~integral(a - 1));
}

inline uint32_t AlignUP(uint32_t numToAlign, uint32_t multiple) {
    assert(multiple && ((multiple & (multiple - 1)) == 0));
    if (multiple == 0)
        return numToAlign;

    int remainder = numToAlign % multiple;
    if (remainder == 0)
        return numToAlign;

    return numToAlign + multiple - remainder;
}

}  // namespace MathUtils

#endif  //MATHUTILS_HPP
