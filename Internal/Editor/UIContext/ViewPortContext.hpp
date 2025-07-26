//
// Created by wpsimon09 on 27/12/24.
//

#ifndef VIEWPORTCONTEXT_HPP
#define VIEWPORTCONTEXT_HPP

#include <imgui.h>

#include "imgui_impl_vulkan.h"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

enum class ViewPortType
{
    eMain,
    eShadowMap,
    ePositionBuffer,
    eMaterial,
    eMainRayTracer,
    eNormalBuffer
};


struct ViewPortContext
{
    int                              width  = 800;
    int                              height = 600;
    std::array<vk::DescriptorSet, 2> ds;
    int                              currentFrameInFlight = 0;
    ApplicationCore::Camera*         camera;
    bool                             hasResized;
    ;

    VkDescriptorSet GetImageDs() { return ds[currentFrameInFlight]; }

    void SetImage(const VulkanCore::VImage2& renderedScene, int frameIndex)
    {
        ds[frameIndex] = ImGui_ImplVulkan_AddTexture(VulkanCore::VSamplers::Sampler2D, renderedScene.GetImageView(),
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        //ds[frameIndex] = ImGui_ImplVulkan_AddTexture(VulkanCore::VSamplers::SamplerClampToEdge, renderedScene.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
};


#endif  //VIEWPORTCONTEXT_HPP
