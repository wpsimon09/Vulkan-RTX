//
// Created by wpsimon09 on 20/12/24.
//

#include "UIContext.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"

namespace VEditor
{
    UIContext::UIContext(const VulkanCore::VDevice& device,
                                       const VulkanCore::VulkanInstance& instance,
                                       const WindowManager& windowManager,
                                       const ApplicationCore::Scene& scene
        ): m_device(device), m_instance(instance), m_windowManager(windowManager), m_scene(scene)
    {
        m_io = nullptr;
        m_viewports[ViewPortType::eMain] = {};
    }

    void UIContext::Initialize(const VulkanCore::VRenderPass& renderPass)
    {
        Utils::Logger::LogInfo("Startin to initialize ImGui...");

        // if i want to have more images like view port material view, i should put it here
        vk::DescriptorPoolSize poolSizes[] ={
          {vk::DescriptorType::eCombinedImageSampler, 1},
          {vk::DescriptorType::eCombinedImageSampler, 1},
          {vk::DescriptorType::eCombinedImageSampler, 1},
        };

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo {};
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolCreateInfo.maxSets = 3;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes));
        descriptorPoolCreateInfo.pPoolSizes = poolSizes;
        m_imguiDescriptorPool = m_device.GetDevice().createDescriptorPool(descriptorPoolCreateInfo);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        m_io = &ImGui::GetIO();
        m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        m_editorFont = m_io->Fonts->AddFontFromFileTTF("Resources/Fonts/roboto/Roboto-Medium.ttf",18);

        m_defaultFont = m_io->Fonts->AddFontDefault();


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
        imGuiVkInitInfo.RenderPass = renderPass.GetRenderPass();
        imGuiVkInitInfo.Subpass = 0;
        imGuiVkInitInfo.MinImageCount = GlobalVariables::MAX_FRAMES_IN_FLIGHT;
        imGuiVkInitInfo.ImageCount = GlobalVariables::MAX_FRAMES_IN_FLIGHT;
        imGuiVkInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&imGuiVkInitInfo);

        Utils::Logger::LogSuccess("ImGui successfully initialized");
    }

    void UIContext::BeginRender()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
        ImGui::PushFont(m_editorFont);
        ImGui::PopFont();
        //ImGui::ShowDemoWindow();
    }

    void UIContext::EndRender()
    {
        ImGui::Render();
        m_imguiDrawData = ImGui::GetDrawData();
    }

    void UIContext::Render(VulkanCore::VCommandBuffer& commandBuffer)
    {
        assert(commandBuffer.GetIsRecording() && "Command buffer that the UI tried to render to is not accepting new commands ");
        ImGui_ImplVulkan_RenderDrawData(m_imguiDrawData, commandBuffer.GetCommandBuffer());
    }

    void UIContext::Destroy()
    {
        Utils::Logger::LogInfoVerboseOnly("Destroying ImGuiInitializer...");
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        m_device.GetDevice().destroyDescriptorPool(m_imguiDescriptorPool);
        Utils::Logger::LogSuccess("ImGui successfully destroyed");
    }

} // VulkanUtils
