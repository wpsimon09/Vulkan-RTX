//
// Created by simon on 13/01/2026.
//

#ifndef VULKAN_RTX_IMGUIIMAGE_HPP
#define VULKAN_RTX_IMGUIIMAGE_HPP
#include "imgui_impl_vulkan.h"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"


#include <array>
#include <vulkan/vulkan.hpp>

namespace VEditor {

class ImGuiImage
{
  public:
    ImGuiImage(const VulkanCore::VImage2& image);

    VkDescriptorSet GetImageDs(int frameInFlight) { return ds[frameInFlight]; }

    void SetImage(const VulkanCore::VImage2& iamge, int frameIndex);

    void OverwriteImage(int frameInFlight, const VulkanCore::VImage2& image, int frameIndex);

  private:
    std::array<vk::DescriptorSet, 2> ds;
};

}  // namespace VEditor

#endif  //VULKAN_RTX_IMGUIIMAGE_HPP
