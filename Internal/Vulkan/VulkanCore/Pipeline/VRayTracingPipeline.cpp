//
// Created by wpsimon09 on 21/04/25.
//

#include "VRayTracingPipeline.hpp"

#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Shader/VRayTracingShaders.hpp"

namespace VulkanCore {
namespace RTX {
VRayTracingPipeline::VRayTracingPipeline(const VulkanCore::VDevice&              device,
                                         const VulkanCore::VRayTracingShaders&   rayTracingShaders,
                                         const VulkanCore::VDescriptorSetLayout& descSetLayout)
    : m_device(device)
    , m_rayTracingShaders(rayTracingShaders)
    , m_descSetLayout(descSetLayout)
{
}

vk::RayTracingPipelineCreateInfoKHR VRayTracingPipeline::Init() {
    // to create the pipeline we first provide all the shader stages it needs
    m_rtxPipelineCreateInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
    m_rtxPipelineCreateInfo.pStages = m_shaderStages.data();

    // indicate what groups those shaders belong to
    m_rtxPipelineCreateInfo.groupCount = m_shaderGroups.size();
    m_rtxPipelineCreateInfo.pGroups = m_shaderGroups.data();

    m_rtxPipelineCreateInfo.maxPipelineRayRecursionDepth = GlobalVariables::RenderingOptions::MaxRecursionDepth;
    m_rtxPipelineCreateInfo.layout = m_pipelineLayout;

    return m_rtxPipelineCreateInfo;
}

void VRayTracingPipeline::CreateCreatePipelineShaders()
{
    vk::PipelineShaderStageCreateInfo stage;
    stage.pName = "main";

    //=====================
    // ray gen
    stage.module           = m_rayTracingShaders.GetShaderModule(RayGen);
    stage.stage            = vk::ShaderStageFlagBits::eRaygenKHR;
    m_shaderStages[RayGen] = stage;

    //=====================
    // miss
    stage.module         = m_rayTracingShaders.GetShaderModule(Miss);
    stage.stage          = vk::ShaderStageFlagBits::eMissKHR;
    m_shaderStages[Miss] = stage;

    //===================================================================================
    // HIT GROUP (groups, any hit shader, closest hit shader and intersection test shader
    // -- closest hit
    stage.module               = m_rayTracingShaders.GetShaderModule(ClosestHit);
    stage.stage                = vk::ShaderStageFlagBits::eClosestHitKHR;
    m_shaderStages[ClosestHit] = stage;
}

void VRayTracingPipeline::CreateShaderHitGroups()
{
    vk::RayTracingShaderGroupCreateInfoKHR groupInfoCI;
    groupInfoCI.anyHitShader       = vk::ShaderUnusedKHR;
    groupInfoCI.closestHitShader   = vk::ShaderUnusedKHR;
    groupInfoCI.generalShader      = vk::ShaderUnusedKHR;
    groupInfoCI.intersectionShader = vk::ShaderUnusedKHR;

    //=========================
    // Ray gene shader group
    // this can only belong to the GENERAL shader grou
    /**
     * SPECS:
     * generalShader is the index of the ray generation, miss, or callable shader
     * from VkRayTracingPipelineCreateInfoKHR::pStages
     */
    groupInfoCI.type          = vk::RayTracingShaderGroupTypeKHR::eGeneral;
    groupInfoCI.generalShader = RayGen;  // index to the shader stage passed to the pipeline as an array
    m_shaderGroups.push_back(groupInfoCI);

    // miss shader (also belongs to the general shader group)
    groupInfoCI.type          = vk::RayTracingShaderGroupTypeKHR::eGeneral;
    groupInfoCI.generalShader = Miss;  // index to the shader stage passed to the pipeline as an array
    m_shaderGroups.push_back(groupInfoCI);

    //reset the general shader goroup since now it is going ot be hit group
    groupInfoCI.type             = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
    groupInfoCI.generalShader    = vk::ShaderUnusedKHR;
    groupInfoCI.closestHitShader = ClosestHit;
    m_shaderGroups.push_back(groupInfoCI);
}
void VRayTracingPipeline::CreatePipelineLayout() {
    Utils::Logger::LogSuccess("Creating pipeline layout...");
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;

    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts    = &m_descSetLayout.GetLayout();

    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges    = nullptr;
    VulkanUtils::Check(
        m_device.GetDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
    Utils::Logger::LogSuccess("Pipeline layout created !");
}

}  // namespace RTX
}  // namespace VulkanCore