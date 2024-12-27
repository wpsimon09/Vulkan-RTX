//
// Created by wpsimon09 on 27/12/24.
//

#ifndef VIEWPORTCONTEXT_HPP
#define VIEWPORTCONTEXT_HPP

#include <imgui.h>

#include "Vulkan/VulkanCore/VImage/VImage.hpp"

enum class ViewPortType
{
    eMain,
    eMaterial
};

struct ViewPortContext
{
    int width = 800;
    int height = 600;
    vk::Image viewPortImage;
};


#endif //VIEWPORTCONTEXT_HPP
