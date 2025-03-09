//
// Created by wpsimon09 on 20/12/24.
//

#include "UIContext.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "ImGuizmo/ImGuizmo.h"
#include "glm/gtc/type_ptr.hpp"

#include "Application/Logger/Logger.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Application/Utils/GizmoUtils.hpp"
#include "Application/Utils/MathUtils.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "IconFontCppHeaders/IconsFontAwesome6.h"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Application/Utils/GizmoUtils.hpp"
#include "Vulkan/Renderer/Renderers/RenderingSystem.hpp"
#include "Vulkan/Renderer/Renderers/UserInterfaceRenderer.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"

namespace VEditor
{
    UIContext::UIContext(const VulkanCore::VDevice& device,
                         const VulkanCore::VulkanInstance& instance,
                         WindowManager& windowManager,
                         Client& client
    ): m_device(device), m_instance(instance), m_windowManager(windowManager),  m_client(client)
    {
        m_io = nullptr;
        m_viewports[ViewPortType::eMain] = {.camera = &m_client.GetCamera()};
    }

    void UIContext::Initialize(const VulkanCore::VSwapChain& swapChain)
    {
        Utils::Logger::LogInfo("Starting to initialize ImGui...");

        // if i want to have more images like view port material view, i should put it here
        vk::DescriptorPoolSize poolSizes[] = {
            {vk::DescriptorType::eCombinedImageSampler, 1},
            {vk::DescriptorType::eCombinedImageSampler, 1},
            {vk::DescriptorType::eCombinedImageSampler, 1},
            {vk::DescriptorType::eCombinedImageSampler, 4}, // for material texture images
        };

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolCreateInfo.maxSets = 7;
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
        SetColourThemePablo();

        std::vector<VkFormat> colourFormats {
            (VkFormat)(swapChain.GetSwapChainFormat())
        };

        VkPipelineRenderingCreateInfo renderingCreateInfo{};
        renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        renderingCreateInfo.colorAttachmentCount = 1;
        renderingCreateInfo.depthAttachmentFormat = static_cast<VkFormat>(m_device.GetDepthFormat());
        renderingCreateInfo.pColorAttachmentFormats = colourFormats.data();


        ImGui_ImplGlfw_InitForVulkan(m_windowManager.GetWindow(), true);
        ImGui_ImplVulkan_InitInfo imGuiVkInitInfo = {};
        imGuiVkInitInfo.Instance = m_instance.GetInstance();
        imGuiVkInitInfo.PhysicalDevice = m_device.GetPhysicalDevice();
        imGuiVkInitInfo.Device = m_device.GetDevice();
        imGuiVkInitInfo.QueueFamily = m_device.GetConcreteQueueFamilyIndex(Graphics);
        imGuiVkInitInfo.Queue = m_device.GetGraphicsQueue();
        imGuiVkInitInfo.PipelineCache = m_imguiPipelineCache;
        imGuiVkInitInfo.DescriptorPool = m_imguiDescriptorPool;
        imGuiVkInitInfo.Subpass = 0;
        imGuiVkInitInfo.MinImageCount = GlobalVariables::MAX_FRAMES_IN_FLIGHT;
        imGuiVkInitInfo.ImageCount = GlobalVariables::MAX_FRAMES_IN_FLIGHT;
        imGuiVkInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        imGuiVkInitInfo.UseDynamicRendering = true;
        imGuiVkInitInfo.PipelineRenderingCreateInfo = renderingCreateInfo;


        ImGui_ImplVulkan_Init(&imGuiVkInitInfo);

        Utils::Logger::LogSuccess("ImGui successfully initialized");
    }

    void UIContext::BeginRender()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        //ImGuizmo::Manipulate(m_client.GetCamera().GetViewMatrix(), m_client.GetCamera().GetViewMatrix(), ImGuizmo::TRANSLATE  )

        ImGui::NewFrame();
        ImGui::PushFont(m_editorFont);
        ImGui::PopFont();

        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);
        //ImGui::ShowDemoWindow();
    }

    void UIContext::EndRender()
    {
        m_selectedSceneNode = m_client.GetScene().GetSelectedSceneNode();

        ApplicationCore::RenderAndUseGizmo(m_selectedSceneNode, m_client.GetCamera().GetViewMatrix(), m_client.GetCamera().GetProjectionMatrix());

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

    void UIContext::SetColourThemePablo()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Corners
        style.WindowRounding = 8.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        // Colors
        colors[ImGuiCol_Text] =                 ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        colors[ImGuiCol_TextDisabled] =         ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] =             ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_ChildBg] =              ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_PopupBg] =              ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_Border] =               ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_BorderShadow] =         ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] =              ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] =       ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_FrameBgActive] =        ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_TitleBg] =              ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_TitleBgActive] =        ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] =     ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
        colors[ImGuiCol_MenuBarBg] =            ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] =          ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] =        ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] =  ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_CheckMark] =            ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
        colors[ImGuiCol_SliderGrab] =           ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] =     ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        colors[ImGuiCol_Button] =               ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_ButtonHovered] =        ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_ButtonActive] =         ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_Header] =               ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_HeaderHovered] =        ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderActive] =         ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_Separator] =            ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] =     ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_SeparatorActive] =      ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_Tab] =                  ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_TabHovered] =           ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_TabActive] =            ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_TabUnfocused] =         ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] =   ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_DockingPreview] =       ImVec4(0.35f, 0.35f, 0.35f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] =       ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] =       ImVec4(0.40f, 0.40f, 0.40f, 0.35f);
        colors[ImGuiCol_NavHighlight] =         ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] =ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] =    ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] =     ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    }
} // VulkanUtils
