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
    CreatePipeline();
}

void VulkanCore::VPipeline::CreatePipeline() {

    CreateShaderStages();

    CreateVertexInputBindingAndAttributes();

    CreatePrimitiveAssembler();

    CreateDynamicViewPort();


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
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    assert(m_device.GetDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout)== vk::Result::eSuccess);

    Utils::Logger::LogSuccess("Created pipeline layout");
}

void VulkanCore::VPipeline::CreateShaderStages() {
    vk::PipelineShaderStageCreateInfo vertexStage;
    vertexStage.stage = vk::ShaderStageFlagBits::eVertex;
    vertexStage.module = m_shaders.GetShaderModule(GlobalVariables::SHADER_TYPE::VERTEX);
    vertexStage.pName = "main";

    vk::PipelineShaderStageCreateInfo fragmentStage;
    fragmentStage.stage = vk::ShaderStageFlagBits::eFragment;
    vertexStage.module = m_shaders.GetShaderModule(GlobalVariables::SHADER_TYPE::FRAGMENT);
    fragmentStage.pName = "main";

    m_shaderStages = {vertexStage, fragmentStage};

}

void VulkanCore::VPipeline::CreateVertexInputBindingAndAttributes() {
    //------------------
    // VERTEX ATTRIBUTES
    //------------------
    VulkanUtils::GetVertexBindingAndAttributeDescription(m_vertexInputBindingDescription, m_vertexInputAttributeDescription);

}

void VulkanCore::VPipeline::CreatePrimitiveAssembler() {
    //--------------------
    // TOPOLOGY
    //--------------------
    m_inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    m_inputAssembly.primitiveRestartEnable = vk::False;
}

void VulkanCore::VPipeline::CreateDynamicViewPort() {
    //--------------------
    // VIEW PORT
    //--------------------
    m_viewport.x = 0.0f;
    m_viewport.y = 0.0f;
    m_viewport.width =(float) m_swapChain.GetExtent().width;
    m_viewport.height =(float) m_swapChain.GetExtent().height;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    //-------------------
    // SCISSORS
    //-------------------
    m_scissor.extent = m_swapChain.GetExtent();
    m_scissor.offset =vk::Offset2D( {0,0});

    //------------------------------
    // DYNAMIC PARTS OF THE PIPELINE
    //------------------------------
    m_dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    m_viewportState.scissorCount = 1;
    m_viewportState.viewportCount = 1;
}

void VulkanCore::VPipeline::CreateDynamicState() {

}
