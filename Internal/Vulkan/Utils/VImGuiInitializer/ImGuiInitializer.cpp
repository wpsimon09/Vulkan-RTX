//
// Created by wpsimon09 on 20/12/24.
//

#include "ImGuiInitializer.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"


namespace VulkanUtils
{
    ImGuiInitializer::ImGuiInitializer(const VulkanCore::VDevice& device, const VulkanCore::VulkanInstance& instance,
                                       const VulkanCore::VRenderPass& renderPass, const WindowManager& windowManager
    ): m_device(device), m_instance(instance), m_renderPass(renderPass), m_windowManager(windowManager)
    {
        m_io = nullptr;
    }

    void ImGuiInitializer::Initialize()
    {
        Utils::Logger::LogInfo("Startin to initialize ImGui...");

        // if i want to have more images like view port material view, i should put it here
        vk::DescriptorPoolSize poolSizes[] ={
          {vk::DescriptorType::eCombinedImageSampler, 1}
        };

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo {};
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes));
        descriptorPoolCreateInfo.pPoolSizes = poolSizes;
        m_imguiDescriptorPool = m_device.GetDevice().createDescriptorPool(descriptorPoolCreateInfo);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        m_io = &ImGui::GetIO();
        m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(m_windowManager.GetWindow(), true);
        ImGui_ImplVulkan_InitInfo vkInitInfo = {};
        vkInitInfo.Instance = m_instance.GetInstance();
        vkInitInfo.PhysicalDevice = m_device.GetPhysicalDevice();
        vkInitInfo.Device = m_device.GetDevice();
        vkInitInfo.QueueFamily = m_device.GetConcreteQueueFamilyIndex(QUEUE_FAMILY_INDEX_GRAPHICS);
        vkInitInfo.Queue = m_device.GetGraphicsQueue();
        vkInitInfo.PipelineCache = m_imguiPipelineCache;
        vkInitInfo.DescriptorPool = m_imguiDescriptorPool;




        Utils::Logger::LogSuccess("ImGui successfully initialized");
    }
} // VulkanUtils
