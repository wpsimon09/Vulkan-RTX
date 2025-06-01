//
// Created by wpsimon09 on 19/01/25.
//

#ifndef INTERSECTIONTESTS_HPP
#define INTERSECTIONTESTS_HPP
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"

namespace ApplicationCore {
bool AABBRayIntersection(Ray& r, VulkanStructs::VBounds* b);
}

#endif  //INTERSECTIONTESTS_HPP
