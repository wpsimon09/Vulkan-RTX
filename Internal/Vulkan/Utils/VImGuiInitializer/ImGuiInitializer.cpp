//
// Created by wpsimon09 on 20/12/24.
//

#include "ImGuiInitializer.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"


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
        m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(m_windowManager.GetWindow(), true);
        ImGui_ImplVulkan_InitInfo imGuiVkInitInfo = {};
        imGuiVkInitInfo.Instance = m_instance.GetInstance();
        imGuiVkInitInfo.PhysicalDevice = m_device.GetPhysicalDevice();
        imGuiVkInitInfo.Device = m_device.GetDevice();
        imGuiVkInitInfo.QueueFamily = m_device.GetConcreteQueueFamilyIndex(QUEUE_FAMILY_INDEX_GRAPHICS);
        imGuiVkInitInfo.Queue = m_device.GetGraphicsQueue();
        imGuiVkInitInfo.PipelineCache = m_imguiPipelineCache;
        imGuiVkInitInfo.DescriptorPool = m_imguiDescriptorPool;
        imGuiVkInitInfo.RenderPass = m_renderPass.GetRenderPass();
        imGuiVkInitInfo.Subpass = 0;
        imGuiVkInitInfo.MinImageCount = GlobalVariables::MAX_FRAMES_IN_FLIGHT;
        imGuiVkInitInfo.ImageCount = GlobalVariables::MAX_FRAMES_IN_FLIGHT;
        imGuiVkInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&imGuiVkInitInfo);

        Utils::Logger::LogSuccess("ImGui successfully initialized");
    }

    void ImGuiInitializer::BeginRender()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiInitializer::EndRender()
    {

    }

    void ImGuiInitializer::Render(VulkanCore::VCommandBuffer& commandBuffer)
    {
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();

        assert(commandBuffer.GetIsRecording() && "Command buffer that the UI tried to render to is not accepting new commands ");
        ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer.GetCommandBuffer());
    }

    void ImGuiInitializer::Destroy()
    {
        Utils::Logger::LogInfoVerboseOnly("Destroying ImGuiInitializer...");
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        m_device.GetDevice().destroyDescriptorPool(m_imguiDescriptorPool);
        Utils::Logger::LogSuccess("ImGui successfully destroyed");
    }
} // VulkanUtils
