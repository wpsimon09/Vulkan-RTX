//
// Created by wpsimon09 on 25/09/24.
//
#pragma once
#ifndef VDEVICE_HPP
#define VDEVICE_HPP

#include <memory>
#include <VMA/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/VMeshDataManager/MeshDataManager.hpp"

namespace VulkanUtils {
class VTransferOperationsManager;
}

namespace VulkanCore {
class VulkanInstance;
class VCommandPool;
}  // namespace VulkanCore

namespace VulkanCore {
struct VQueueFamilyIndices
{
    std::optional<std::pair<EQueueFamilyIndexType, uint32_t>> graphicsFamily;
    std::optional<std::pair<EQueueFamilyIndexType, uint32_t>> presentFamily;
    std::optional<std::pair<EQueueFamilyIndexType, uint32_t>> computeFamily;
    std::optional<std::pair<EQueueFamilyIndexType, uint32_t>> transferFamily;

    bool isComplete() const
    {
        return (graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value())
               || computeFamily.has_value();
    };
};


VQueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& physicalDevice, const VulkanCore::VulkanInstance& instance);


class VDevice : public VObject
{
  public:
    VDevice(const VulkanCore::VulkanInstance& instance);

    //----------------------------------------------------------------------------------------
    //   GETTERS
    //----------------------------------------------------------------------------------------
    const vk::PhysicalDevice&     GetPhysicalDevice() const { return m_physicalDevice; }
    VulkanCore::VCommandPool&     GetTransferCommandPool() const;
    VulkanCore::VCommandPool&     GetSingleThreadCommandPool() const { return *m_transferCommandPoolForSingleThread; }
    const vk::Device&             GetDevice() const { return m_device; }
    VulkanCore::MeshDatatManager& GetMeshDataManager() const;
    ;
    const VQueueFamilyIndices&               GetQueueFamilyIndices() const { return m_queueFamilyIndices; };
    const VmaAllocator&                      GetAllocator() const { return m_vmaAllocator; };
    const vk::Queue&                         GetGraphicsQueue() const { return m_graphicsQueue; }
    const vk::Queue&                         GetComputeQueue() const { return m_computeQueue; }
    const vk::Queue&                         GetTransferQueue() const { return m_transferQueue; }
    const vk::Queue&                         GetPresentQueue() const { return m_presentQueue; }
    const vk::Format&                        GetDepthFormat() const { return m_depthFormat; }
    const vk::SampleCountFlagBits            GetSampleCount() const { return m_sampleCount; }
    VmaTotalStatistics&                      GetDeviceStatistics() { return m_vmaStatistics; }
    VulkanUtils::VTransferOperationsManager& GetTransferOpsManager() const { return *m_transferOpsManager; }
    //----------------------------------------------------------------------------------------

    void UpdateSampleCount();

    const uint32_t&   GetConcreteQueueFamilyIndex(EQueueFamilyIndexType queueFamilyType) const;
    const std::string GetQueueFamilyString(EQueueFamilyIndexType queueFamilyType) const;

    void UpdateMemoryStatistics();

    mutable std::mutex DeviceMutex;

    void Destroy() override;

    vk::detail::DispatchLoaderDynamic DispatchLoader;

  private:
    vk::PhysicalDevice m_physicalDevice;
    vk::Device         m_device;  //logical device

    vk::Queue m_graphicsQueue;
    vk::Queue m_computeQueue;
    vk::Queue m_transferQueue;
    vk::Queue m_presentQueue;

    vk::Format              m_depthFormat;
    vk::SampleCountFlagBits m_sampleCount;

    std::array<std::unique_ptr<VulkanCore::VCommandPool>, GlobalVariables::MAX_THREADS> m_transferCommandPool;
    std::unique_ptr<VulkanCore::VCommandPool>                m_transferCommandPoolForSingleThread;
    std::unique_ptr<VulkanUtils::VTransferOperationsManager> m_transferOpsManager;
    std::unique_ptr<MeshDatatManager>                        m_meshDataManager;

    VQueueFamilyIndices m_queueFamilyIndices;

    const VulkanCore::VulkanInstance& m_instance;

    VmaAllocator       m_vmaAllocator;
    VmaTotalStatistics m_vmaStatistics;
    VmaStatistics      __mode;


  private:
    vk::PhysicalDevice PickPhysicalDevice();
    void               CreateLogicalDevice();
    void               CreateVmaAllocator(const VulkanCore::VulkanInstance& instance);
    void               RetreiveDepthFormat();
    void               FetchMaxSampleCount();
};

}  // namespace VulkanCore

#endif