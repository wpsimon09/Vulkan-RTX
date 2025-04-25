//
// Created by wpsimon09 on 20/04/25.
//

#include "VRayTracingDataManager.hpp"

#include "VulkanRtx.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhr.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingBuilderKhrHelpers.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "vulkan/vulkan.hpp"

namespace VulkanUtils {
VRayTracingDataManager::VRayTracingDataManager(const VulkanCore::VDevice& device)
    : m_device(device)
{
    m_rayTracingBuilder = std::make_unique<VulkanCore::RTX::VRayTracingBuilderKHR>(device);
}

void VRayTracingDataManager::UpdateAS() {

}

void VRayTracingDataManager::InitAs(std::vector<VulkanCore::RTX::BLASInput>& blasInputs)
{
    m_instances.clear();
    m_blasInputs.clear();
    m_instances.shrink_to_fit();
    m_blasInputs.shrink_to_fit();
    m_rayTracingBuilder->Clear();
    m_rayTracingBuilder->BuildBLAS(blasInputs, vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace
                                                   | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction);

    // single shader hit group is going to be stored under this index...
    std::vector<vk::AccelerationStructureInstanceKHR> instances;
    instances.reserve(blasInputs.size());

    int i                   = 0;
    int shaderHitGroupIndex = 0;
    // for now every instance will be every BLAS, i will have to later redo how scene is describing the
    for(auto& instance : blasInputs)
    {
        vk::AccelerationStructureInstanceKHR instanceInfo{};
        instanceInfo.transform           = VulkanCore::RTX::GlmToMatrix4KHR(instance.transform);
        instanceInfo.instanceCustomIndex = i;

        instanceInfo.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instanceInfo.accelerationStructureReference         = m_rayTracingBuilder->GetInstanceDeviceAddress(i);
        instanceInfo.mask                                   = 0xFF;
        instanceInfo.instanceShaderBindingTableRecordOffset = 0;

        m_instances.emplace_back(instanceInfo);
        i++;
    }
    m_rayTracingBuilder->BuildTLAS(m_instances);
}
void VRayTracingDataManager::Destroy()
{
    m_rayTracingBuilder->Destroy();
}
vk::WriteDescriptorSetAccelerationStructureKHR VRayTracingDataManager::GetTlasWriteInfo() {
    vk::WriteDescriptorSetAccelerationStructureKHR asWrite;
    asWrite.accelerationStructureCount = 1;
    asWrite.pAccelerationStructures = &m_rayTracingBuilder->GetTLAS();

    return  asWrite ;
}

} // VulkanUtils