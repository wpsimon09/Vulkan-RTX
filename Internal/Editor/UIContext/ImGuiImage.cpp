//
// Created by simon on 13/01/2026.
//

#include "ImGuiImage.hpp"

namespace VEditor {
ImGuiImage::ImGuiImage(const VulkanCore::VImage2& image)
{
    for(int currentFrame = 0; currentFrame < GlobalVariables::MAX_FRAMES_IN_FLIGHT; currentFrame++)
    {
        SetImage(image, currentFrame);
    }
}
void ImGuiImage::SetImage(const VulkanCore::VImage2& iamge, int frameIndex)
{
    {
        ds[frameIndex] = ImGui_ImplVulkan_AddTexture(VulkanCore::VSamplers::Sampler2D, iamge.GetImageView(),
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}
void ImGuiImage::OverwriteImage(int frameInFlight, const VulkanCore::VImage2& image, int frameIndex)
{
    {
        ImGui_ImplVulkan_UpdateTexture(ds[frameInFlight], VulkanCore::VSamplers::Sampler2D, image.GetImageView(),
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}
}  // namespace VEditor