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
#include "IconFontCppHeaders/IconsFontAwesome6.h"
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
        vk::DescriptorPoolSize poolSizes[] = {
            {vk::DescriptorType::eCombinedImageSampler, 1},
            {vk::DescriptorType::eCombinedImageSampler, 1},
            {vk::DescriptorType::eCombinedImageSampler, 1},
        };

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
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


        ImFontConfig fontConfig;
        fontConfig.MergeMode = true;
        fontConfig.GlyphMinAdvanceX = 13.0f;

        static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};

        m_defaultFont = m_io->Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto-Medium.ttf", 16.0f);

        m_editorFont = m_io->Fonts->AddFontFromFileTTF("Resources/Fonts/fontawesome/fa-solid-900.ttf", 16.0f,
                                                       &fontConfig, icon_ranges);

        ImGui::StyleColorsDark();
        ThemeConfig();

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
        assert(
            commandBuffer.GetIsRecording() &&
            "Command buffer that the UI tried to render to is not accepting new commands ");
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

    void UIContext::ThemeConfig()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Rounded corners
        style.WindowRounding = 8.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        // Colors
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.43f, 0.50f, 0.56f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.50f, 0.56f, 0.50f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.56f, 1.00f, 0.80f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.22f, 0.36f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }
} // VulkanUtils
