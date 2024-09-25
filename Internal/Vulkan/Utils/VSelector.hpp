//
// Created by wpsimon09 on 25/09/24.
//
#pragma once
#ifndef VSELECTOR_HPP
#define VSELECTOR_HPP

#include <vulkan/vulkan.hpp>

namespace VulkanUtils::Selector
{
    bool IsDeviceSuitable(const vk::PhysicalDevice& physicalDevice);
}


#endif //VSELECTOR_HPP
