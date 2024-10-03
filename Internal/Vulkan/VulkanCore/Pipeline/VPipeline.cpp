//
// Created by wpsimon09 on 03/10/24.
//

#include "VPipeline.hpp"

#include "Vulkan/VulkanCore/Shader/VShader.hpp"

VulkanCore::VPipeline::VPipeline(const VulkanCore::VDevice &device, const VulkanCore::VSwapChain &swapChain,
                                 const VulkanCore::VShader &shaders):m_device(device), m_swapChain(swapChain), m_shaders(shaders) {
    CreatePipeline();
}

void VulkanCore::VPipeline::CreatePipeline() {
    vk::PipelineShaderStageCreateInfo shaderStages[2];

    vk::PipelineShaderStageCreateInfo vertexStage;
    vertexStage.stage = vk::ShaderStageFlagBits::eVertex;
    vertexStage.module = m_shaders.GetShaderModule(GlobalVariables::SHADER_TYPE::VERTEX);
    vertexStage.pName = "main";



    vk::PipelineShaderStageCreateInfo fragmentStage;
    fragmentStage.stage = vk::ShaderStageFlagBits::eFragment;
    vertexStage.module = m_shaders.GetShaderModule(GlobalVariables::SHADER_TYPE::FRAGMENT);
    fragmentStage.pName = "main";

}
