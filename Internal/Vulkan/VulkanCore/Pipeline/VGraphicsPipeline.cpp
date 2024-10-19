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


VulkanCore::VGraphicsPipeline::VGraphicsPipeline(const VulkanCore::VDevice &device, const VulkanCore::VSwapChain &swapChain,
    const VulkanCore::VShader &shaders, const VulkanCore::VRenderPass &renderPass)
        : m_device{device}, m_swapChain{swapChain}, m_shaders{shaders},m_renderPass{renderPass}, VObject()
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

    commandBuffer.BeginRecording();
    for (auto &command : m_pipelineCommands) {
        command(commandBuffer.GetCommandBuffer());
    }
    commandBuffer.EndRecording();
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
    info.pDepthStencilState = nullptr;
    info.pColorBlendState = &m_colorBlendState;
    info.pDynamicState = &m_dynamicStateInfo;
    info.layout = m_pipelineLayout;
    info.renderPass = m_renderPass.GetRenderPass();
    //---------------------------------------


    info.pNext = nullptr;
    info.basePipelineIndex = -1;

    return info;

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

    //-------------------
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
    m_multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    m_multisampling.minSampleShading = 1.0f;
    m_multisampling.pSampleMask = nullptr;
    m_multisampling.alphaToCoverageEnable = VK_FALSE;
    m_multisampling.alphaToOneEnable = VK_FALSE;
}

void VulkanCore::VGraphicsPipeline::CreateDepthStencil() {
    m_depthStencil.depthTestEnable = vk::False;
    m_depthStencil.depthWriteEnable = vk::False;
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

void VulkanCore::VGraphicsPipeline::CreatePipelineLayout(vk::DescriptorSetLayout* descriptorSet,  int descriptorCounts) {
    Utils::Logger::LogSuccess("Creating pipeline layout...");
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.setLayoutCount = descriptorCounts;
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSet;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    assert(m_device.GetDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) == vk::Result::eSuccess);
    Utils::Logger::LogSuccess("Pipeline layout created !");
}






