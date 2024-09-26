//
// Created by wpsimon09 on 26/09/24.
//

#ifndef GLOBALSTRUCTS_HPP
#define GLOBALSTRUCTS_HPP

#include <vulkan/vulkan.hpp>

struct DesiredDeviceFeatures
{
    vk::PhysicalDeviceType deviceType;
    vk::PhysicalDeviceFeatures deviceFeatures;
};

#endif //GLOBALSTRUCTS_HPP
