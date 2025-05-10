//
// Created by wpsimon09 on 21/04/25.
//

#include "VRayTracingPipeline.hpp"

#include "Application/Utils/MathUtils.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Shader/VRayTracingShaders.hpp"

namespace VulkanCore {
namespace RTX {
VRayTracingPipeline::VRayTracingPipeline(const VulkanCore::VDevice&              device,
VulkanCore::VRayTracingShaders&   rayTracingShaders,
                                         const VulkanCore::VDescriptorSetLayout& descSetLayout)
    : m_device(device)
    , m_rayTracingShaders(rayTracingShaders)
    , m_descSetLayout(descSetLayout)
{
}

void VRayTracingPipeline::Init()
{
    CreatePipelineLayout();
    CreateCreatePipelineShaders();
    CreateShaderHitGroups();

    // to create the pipeline we first provide all the shader stages it needs
    m_rtxPipelineCreateInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
    m_rtxPipelineCreateInfo.pStages    = m_shaderStages.data();

    // indicate what groups those shaders belong to
    m_rtxPipelineCreateInfo.groupCount = m_shaderGroups.size();
    m_rtxPipelineCreateInfo.pGroups    = m_shaderGroups.data();

    assert(GlobalVariables::RenderingOptions::MaxRecursionDepth <= GlobalVariables::GlobalStructs::RayTracingPipelineProperties.maxRayRecursionDepth);
    m_rtxPipelineCreateInfo.maxPipelineRayRecursionDepth = GlobalVariables::GlobalStructs::RayTracingPipelineProperties.maxRayRecursionDepth;
    m_rtxPipelineCreateInfo.layout                       = m_pipelineLayout;
}
const vk::PipelineLayout& VRayTracingPipeline::GetPipelineLayout()
{
    return m_pipelineLayout;
}
void VRayTracingPipeline::Destroy()
{
    m_device.GetDevice().destroyPipelineLayout(m_pipelineLayout);
    //m_shaderBindingTable->Destroy();
    m_shaderBindingTable->DestroyStagingBuffer();
    m_device.GetDevice().destroyPipeline(m_rtPipelineHandle);

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

    //======================
    // miss - shadow
    stage.module = m_rayTracingShaders.GetShaderModule(MissShadow);
    stage.stage = vk::ShaderStageFlagBits::eMissKHR;
    m_shaderStages[MissShadow] = stage;

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

    // miss shadow shader (also belongs to the general shader group)
    groupInfoCI.type          = vk::RayTracingShaderGroupTypeKHR::eGeneral;
    groupInfoCI.generalShader = MissShadow;  // index to the shader stage passed to the pipeline as an array
    m_shaderGroups.push_back(groupInfoCI);

    //reset the general shader goroup since now it is going ot be hit group
    groupInfoCI.type             = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
    groupInfoCI.generalShader    = vk::ShaderUnusedKHR;
    groupInfoCI.closestHitShader = ClosestHit;
    m_shaderGroups.push_back(groupInfoCI);
}
void VRayTracingPipeline::CreatePipelineLayout()
{
    Utils::Logger::LogSuccess("Creating pipeline layout...");
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;

    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts    = &m_descSetLayout.GetLayout();

    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges    = nullptr;
    VulkanUtils::Check(m_device.GetDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
    Utils::Logger::LogSuccess("Pipeline layout created !");
}
void VRayTracingPipeline::CreateShaderBindingTable()
{
    uint32_t hitCount    = 1;
    uint32_t missCount   = 2;
    auto     handleCount = hitCount + missCount + 1;  // 1 is for ray gen which is ALLWAYS one
    uint32_t handleSize  = GlobalVariables::GlobalStructs::RayTracingPipelineProperties.shaderGroupHandleSize;

    // this needs to tbe aligned, DONT FORGET that each entry in the shader binding table
    // is a shader group can be ray gan, hit or any other shader group
    // this stride is specifing into what row can we go
    // e.g: 4 * stride is going to be 3rd row
    uint32_t handleSizeAligned =
        MathUtils::align_up(handleSize, GlobalVariables::GlobalStructs::RayTracingPipelineProperties.shaderGroupHandleAlignment);

    //======================================================================================
    // specify the size and strides for the shaders so that we know what should be looked up
    m_rGenRegion.stride =
        MathUtils::align_up(handleSizeAligned, GlobalVariables::GlobalStructs::RayTracingPipelineProperties.shaderGroupBaseAlignment);
    m_rGenRegion.size = m_rGenRegion.stride;

    m_rMissRegion.stride = handleSizeAligned;
    m_rMissRegion.size = MathUtils::align_up(missCount * handleSizeAligned,GlobalVariables::GlobalStructs::RayTracingPipelineProperties.shaderGroupBaseAlignment);

    m_rHitRegion.stride = handleSizeAligned;
    m_rHitRegion.size = MathUtils::align_up(hitCount * handleSizeAligned,GlobalVariables::GlobalStructs::RayTracingPipelineProperties.shaderGroupBaseAlignment);

    //==========================================
    // retrieve handles for shaders form Vulkan
    uint32_t dataSize = handleCount * handleSize;
    std::vector<uint8_t> handles(dataSize);
    assert(m_rtPipelineHandle != nullptr && "Make sure you have build the pipeline from the Effect inteface");

    auto result = m_device.GetDevice().getRayTracingShaderGroupHandlesKHR(m_rtPipelineHandle, 0, handleCount, handleSize, handles.data(), m_device.DispatchLoader);
    VulkanUtils::Check(result);

    //===========================================
    // allocate buffer for shader binding table
    vk::DeviceSize sbtSize = m_rGenRegion.size + m_rMissRegion.size + m_rHitRegion.size + m_rCallRegion.size;

    m_shaderBindingTable = std::make_unique<VulkanCore::VBuffer>(m_device, "Shader binding table");
    m_shaderBindingTable->CreateHostVisibleBuffer(sbtSize, static_cast<VkBufferUsageFlags>(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress
                                                     | vk::BufferUsageFlagBits::eShaderBindingTableKHR));

    m_rGenRegion.deviceAddress = m_shaderBindingTable->GetBufferAdress();
    m_rMissRegion.deviceAddress = m_shaderBindingTable->GetBufferAdress() + m_rGenRegion.size;
    m_rHitRegion.deviceAddress = m_shaderBindingTable->GetBufferAdress() + m_rGenRegion.size + m_rMissRegion.size;

    // helper to retrieve the handle data pointer so that we can copy the shader to it
    auto getHandle = [&](int i) { return handles.data() + i * handleSize; };

    //map the SBT buffer so that we can copy to it
    m_shaderBindingTable->MapStagingBuffer();
    auto* pSBTBuffer = reinterpret_cast<uint8_t*>(m_shaderBindingTable->GetMapPointer());
    uint8_t* pData = {nullptr};
    uint32_t handleIndex = 0;

    // Ray gen
    pData = pSBTBuffer;
    memcpy(pData, getHandle(handleIndex++),handleSize);

    // miss shaders
    pData  = pSBTBuffer + m_rGenRegion.size;
    for (int i = 0; i<missCount; i++) {
        memcpy(pData, getHandle(handleIndex++),handleSize);
        pData += m_rMissRegion.stride;
    }

    // hit shaders
    pData = pSBTBuffer + m_rGenRegion.size + m_rMissRegion.size;
    for (int i = 0; i<hitCount; i++) {
        memcpy(pData, getHandle(handleIndex++),handleSize);
        pData += m_rHitRegion.stride;
    }

    m_shaderBindingTable->UnMapStagingBuffer();


}


}  // namespace RTX
}  // namespace VulkanCore