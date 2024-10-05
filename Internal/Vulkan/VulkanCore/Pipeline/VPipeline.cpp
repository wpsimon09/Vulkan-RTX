//
// Created by wpsimon09 on 03/10/24.
//

#include "VPipeline.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"

VulkanCore::VPipeline::VPipeline(const VulkanCore::VDevice &device, const VulkanCore::VSwapChain &swapChain,
                                 const VulkanCore::VShader &shaders):m_device(device), m_swapChain(swapChain), m_shaders(shaders) {
}

void VulkanCore::VPipeline::CreatePipeline() {
    vk::PipelineShaderStageCreateInfo vertexStage;
    vertexStage.stage = vk::ShaderStageFlagBits::eVertex;
    vertexStage.module = m_shaders.GetShaderModule(GlobalVariables::SHADER_TYPE::VERTEX);
    vertexStage.pName = "main";

    vk::PipelineShaderStageCreateInfo fragmentStage;
    fragmentStage.stage = vk::ShaderStageFlagBits::eFragment;
    vertexStage.module = m_shaders.GetShaderModule(GlobalVariables::SHADER_TYPE::FRAGMENT);
    fragmentStage.pName = "main";

    std::array<vk::PipelineShaderStageCreateInfo,3> shaderStages = {vertexStage, fragmentStage};



    //------------------
    // VERTEX ATTRIBUTES
    //------------------
    vk::VertexInputBindingDescription bindingDescription = {};
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {};

    VulkanUtils::GetVertexBindingAndAttributeDescription(bindingDescription, attributeDescriptions);

    //--------------------
    // TOPOLOGY
    //--------------------
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = vk::False;

    //--------------------
    // TOPOLOGY
    //--------------------
    vk::Viewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width =(float) m_swapChain.GetExtent().width;
    viewport.height =(float) m_swapChain.GetExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //-------------------
    // SCISSORS
    //-------------------
    vk::Rect2D scissors = {};
    scissors.extent = m_swapChain.GetExtent();
    scissors.offset =vk::Offset2D( {0,0});

    //------------------------------
    // DYNAMIC PARTS OF THE PIPELINE
    //------------------------------
    std::vector<vk::DynamicState> dynamicState = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    //--------------------------------------
    // CREATING DYNAMIC PIPELINE INFO STATES
    //--------------------------------------
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicState.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicState.data();

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;

    //--------------------------
    // RASTERIZER
    //--------------------------
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.depthClampEnable = vk::False;
    rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
    rasterizationStateCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0F;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    //-----------------------
    // MULTISAMPLING
    //-----------------------
    vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo{};
    multisampleCreateInfo.sampleShadingEnable = vk::False;
    multisampleCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampleCreateInfo.minSampleShading = 1.0f;
    multisampleCreateInfo.pSampleMask = nullptr;
    multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

    //------------------
    // DEPTH AND STENCIL
    //------------------
    //!

    //--------------------
    // COLOUR BLENDING
    //--------------------
    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA;
    colorBlendAttachmentState.blendEnable = vk::False;
    colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
    colorBlendStateCreateInfo.logicOpEnable = vk::False;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

    //-------------------------
    // PIPELINE LAYOUT
    //-------------------------
}
