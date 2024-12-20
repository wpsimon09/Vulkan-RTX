//
// Created by wpsimon09 on 20/12/24.
//

#include "ImGuiInitializer.hpp"

namespace VulkanUtils {
    ImGuiInitializer::ImGuiInitializer(const VulkanCore::VDevice& device, const VulkanCore::VulkanInstance& instance,
        const VulkanCore::VRenderPass& renderPass): m_device(device), m_instance(instance), m_renderPass(renderPass),
    m_io(ImGui::GetIO())
    {

    }

    void ImGuiInitializer::Initialize()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }
} // VulkanUtils