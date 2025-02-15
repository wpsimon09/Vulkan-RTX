//
// Created by wpsimon09 on 03/10/24.
//

#include "VGraphicsPipeline.hpp"

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include <vulkan/vulkan.hpp>


VulkanCore::VGraphicsPipeline::VGraphicsPipeline(const VulkanCore::VDevice &device, const VulkanCore::VSwapChain &swapChain,
    const VulkanCore::VShader &shaders, const VulkanCore::VRenderPass &renderPass, const VulkanCore::VDescriptorSetLayout &descriptorLayout)
        : VObject(), m_shaders(shaders), m_device(device),m_swapChain(swapChain), m_renderPass(renderPass), m_descriptorSetLayout(descriptorLayout)
{}



void VulkanCore::VGraphicsPipeline::Init() {

    CreateShaderStages();
    CreateVertexInputBindingAndAttributes();
    CreatePrimitiveAssembler();
    CreateDynamicViewPort();
    CreateDynamicState();
    CreateRasterizer();
    CreateMultisampling();
    CreateDepthStencil();
    CreateColorBlend();
    CreatePipelineLayout();
}

void VulkanCore::VGraphicsPipeline::Destroy() {
    m_device.GetDevice().destroyPipelineLayout(m_pipelineLayout);
}

const void VulkanCore::VGraphicsPipeline::RecordPipelineCommands(VulkanCore::VCommandBuffer &commandBuffer) const {
    for (auto &command : m_pipelineCommands) {
        command(commandBuffer.GetCommandBuffer());
    }
}

const void VulkanCore::VGraphicsPipeline::AddCommand(const Command &command) {
    m_pipelineCommands.emplace_back(command);
}


const vk::GraphicsPipelineCreateInfo VulkanCore::VGraphicsPipeline::GetGraphicsPipelineCreateInfoStruct() const {
    vk::GraphicsPipelineCreateInfo info = {};

    //----------------------------------------
    // SHADERS
    // ---------------------------------------
    info.stageCount = m_shaderStages.size();
    info.pStages = m_shaderStages.data();
    //---------------------------------------

    //----------------------------------------
    // FIXED FUNCTIONALITY
    // ---------------------------------------
    info.pVertexInputState = &m_vertexInputState;
    info.pInputAssemblyState = &m_inputAssembly;
    info.pViewportState = &m_viewportState;
    info.pRasterizationState = &m_rasterizer;
    info.pMultisampleState = &m_multisampling;
    info.pDepthStencilState = &m_depthStencil;
    info.pColorBlendState = &m_colorBlendState;
    info.pDynamicState = &m_dynamicStateInfo;
    info.layout = m_pipelineLayout;
    info.renderPass = m_renderPass.GetRenderPass();
    //---------------------------------------


    info.pNext = nullptr;
    info.basePipelineIndex = -1;

    return info;

}

const vk::PipelineLayout & VulkanCore::VGraphicsPipeline::GetPipelineLayout() const {
    return m_pipelineLayout;
}

void VulkanCore::VGraphicsPipeline::CreateShaderStages() {
    vk::PipelineShaderStageCreateInfo vertexStage;
    vertexStage.stage = vk::ShaderStageFlagBits::eVertex;
    vertexStage.module = m_shaders.GetShaderModule(GlobalVariables::SHADER_TYPE::VERTEX);
    vertexStage.pName = "main";

    vk::PipelineShaderStageCreateInfo fragmentStage;
    fragmentStage.stage = vk::ShaderStageFlagBits::eFragment;
    fragmentStage.module = m_shaders.GetShaderModule(GlobalVariables::SHADER_TYPE::FRAGMENT);
    fragmentStage.pName = "main";

    m_shaderStages = {vertexStage, fragmentStage};

}

void VulkanCore::VGraphicsPipeline::CreateVertexInputBindingAndAttributes() {
    VulkanUtils::GetVertexBindingAndAttributeDescription(m_vertexInputBindingDescription, m_vertexInputAttributeDescription);
    m_vertexInputState.vertexAttributeDescriptionCount = m_vertexInputAttributeDescription.size();
    m_vertexInputState.vertexBindingDescriptionCount = 1;

    m_vertexInputState.pVertexAttributeDescriptions = m_vertexInputAttributeDescription.data();
    m_vertexInputState.pVertexBindingDescriptions = &m_vertexInputBindingDescription;
}

void VulkanCore::VGraphicsPipeline::CreatePrimitiveAssembler() {
    m_inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    m_inputAssembly.primitiveRestartEnable = vk::False;
}

void VulkanCore::VGraphicsPipeline::CreateDynamicViewPort() {
    //--------------------
    // VIEW PORT
    //--------------------
    m_viewport.x = 0.0f;
    m_viewport.y = 0.0f;
    m_viewport.width =(float) m_swapChain.GetExtent().width;
    m_viewport.height =(float) m_swapChain.GetExtent().height;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    //-------------------P
    // SCISSORS
    //-------------------
    m_scissor.extent = m_swapChain.GetExtent();
    m_scissor.offset =vk::Offset2D(0,0);


    m_viewportState.scissorCount = 1;
    m_viewportState.viewportCount = 1;
}

void VulkanCore::VGraphicsPipeline::CreateDynamicState() {

    //------------------------------
    // DYNAMIC PARTS OF THE PIPELINE
    //------------------------------
    m_dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    m_dynamicStateInfo.dynamicStateCount = m_dynamicStates.size();
    m_dynamicStateInfo.pDynamicStates = m_dynamicStates.data();
}

void VulkanCore::VGraphicsPipeline::CreateRasterizer() {
    m_rasterizer.depthClampEnable = vk::False;
    m_rasterizer.polygonMode = vk::PolygonMode::eFill;
    m_rasterizer.rasterizerDiscardEnable = VK_FALSE;
    m_rasterizer.lineWidth = 1.0f;
    m_rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    m_rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    m_rasterizer.depthBiasEnable = VK_FALSE;
    m_rasterizer.depthBiasConstantFactor = 0.0F;
    m_rasterizer.depthBiasClamp = 0.0f;
    m_rasterizer.depthBiasSlopeFactor = 0.0f;
}

void VulkanCore::VGraphicsPipeline::CreateMultisampling() {
    m_multisampling.sampleShadingEnable = vk::False;
    m_multisampling.rasterizationSamples = m_device.GetSampleCount();
    m_multisampling.minSampleShading = 1.0f;
    m_multisampling.pSampleMask = nullptr;
    m_multisampling.alphaToCoverageEnable = VK_FALSE;
    m_multisampling.alphaToOneEnable = VK_FALSE;
}

void VulkanCore::VGraphicsPipeline::CreateDepthStencil() {
    m_depthStencil.depthTestEnable = vk::True;
    m_depthStencil.depthWriteEnable = vk::True;
    m_depthStencil.depthCompareOp = vk::CompareOp::eLess;
    m_depthStencil.depthBoundsTestEnable = vk::False;
    m_depthStencil.minDepthBounds = 0.0f;
    m_depthStencil.maxDepthBounds = 1.0f;
    m_depthStencil.stencilTestEnable = vk::False;

}

void VulkanCore::VGraphicsPipeline::CreateColorBlend() {
    //--------------------
    // COLOUR BLENDING
    //--------------------
    m_colorBlendAttachmentState.colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA;
    m_colorBlendAttachmentState.blendEnable = vk::False;
    m_colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eOne;
    m_colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eZero;
    m_colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
    m_colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    m_colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    m_colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
    m_colorBlendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;

    m_colorBlendState.logicOpEnable = VK_FALSE;
    m_colorBlendState.logicOp = vk::LogicOp::eCopy;
    m_colorBlendState.logicOpEnable = vk::False;
    m_colorBlendState.attachmentCount = 1;
    m_colorBlendState.pAttachments = &m_colorBlendAttachmentState;
    m_colorBlendState.blendConstants[0] = 0.0f;
    m_colorBlendState.blendConstants[1] = 0.0f;
    m_colorBlendState.blendConstants[2] = 0.0f;
    m_colorBlendState.blendConstants[3] = 0.0f;

}

void VulkanCore::VGraphicsPipeline::CreatePipelineLayout() {
    Utils::Logger::LogSuccess("Creating pipeline layout...");
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;

    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout.GetLayout();

    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    assert(m_device.GetDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) == vk::Result::eSuccess);
    Utils::Logger::LogSuccess("Pipeline layout created !");
}

void VulkanCore::VGraphicsPipeline::EnableBlendingAlpha(){
    m_colorBlendAttachmentState.blendEnable = vk::True;
    m_colorBlendAttachmentState.colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA;
    m_colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    m_colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOne;
    m_colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
    m_colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    m_colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    m_colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;

    m_colorBlendState.logicOpEnable = vk::True;
    m_colorBlendState.logicOp = vk::LogicOp::eCopy;
    m_colorBlendState.logicOpEnable = vk::True;
    m_colorBlendState.attachmentCount = 1;
    m_colorBlendState.pAttachments = &m_colorBlendAttachmentState;
    m_colorBlendState.blendConstants[0] = 0.0f;
    m_colorBlendState.blendConstants[1] = 0.0f;
    m_colorBlendState.blendConstants[2] = 0.0f;
    m_colorBlendState.blendConstants[3] = 0.0f;
}

void VulkanCore::VGraphicsPipeline::EnableBlendingAdditive(){
    m_colorBlendAttachmentState.blendEnable = vk::True;
    m_colorBlendAttachmentState.colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA;
    m_colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    m_colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    m_colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
    m_colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    m_colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    m_colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;

    m_colorBlendState.logicOpEnable = vk::True;
    m_colorBlendState.logicOp = vk::LogicOp::eCopy;
    m_colorBlendState.logicOpEnable = vk::True;
    m_colorBlendState.attachmentCount = 1;
    m_colorBlendState.pAttachments = &m_colorBlendAttachmentState;
    m_colorBlendState.blendConstants[0] = 0.0f;
    m_colorBlendState.blendConstants[1] = 0.0f;
    m_colorBlendState.blendConstants[2] = 0.0f;
    m_colorBlendState.blendConstants[3] = 0.0f;
}






