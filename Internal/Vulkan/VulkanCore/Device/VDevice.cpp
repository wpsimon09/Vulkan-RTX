//
// Created by wpsimon09 on 25/09/24.
//

#include "VDevice.hpp"

#include <set>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/GlobalStructs.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandPool.hpp"
#include "Vulkan/VulkanCore/Instance/VInstance.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"
#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"

VulkanCore::VQueueFamilyIndices VulkanCore::FindQueueFamilies(const vk::PhysicalDevice&         physicalDevice,
                                                              const VulkanCore::VulkanInstance& instance)
{
    VulkanCore::VQueueFamilyIndices indices;

    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    ;

    //------------------------
    // NORMAL QUEUE FAMILIES
    //------------------------
    indices.graphicsFamily =
        std::make_pair(Graphics, VulkanUtils::FindQueueFamily(queueFamilyProperties, vk::QueueFlagBits::eGraphics));
    indices.transferFamily =
        std::make_pair(Transfer, VulkanUtils::FindQueueFamily(queueFamilyProperties, vk::QueueFlagBits::eTransfer));
    indices.computeFamily =
        std::make_pair(Compute, VulkanUtils::FindQueueFamily(queueFamilyProperties, vk::QueueFlagBits::eCompute));

    //----------------------
    // PRESENT QUEUE FAMILY
    //----------------------
    vk::Bool32   presentSupport     = false;
    unsigned int presentFamilyIndex = 0;
    for(int i = 0; i < queueFamilyProperties.size(); i++)
    {
        presentSupport = physicalDevice.getSurfaceSupportKHR(i, instance.GetSurface());
        if(presentSupport)
        {
            presentFamilyIndex = i;
        }
        break;
    }

    indices.presentFamily = std::make_pair(EQueueFamilyIndexType::PresentKHR, presentFamilyIndex);
    assert(presentSupport == true);
    Utils::Logger::LogInfoVerboseOnly("Found transfer queue family at index: "
                                      + std::to_string(indices.presentFamily.value().second));

    return indices;
}


vk::PhysicalDevice VulkanCore::VDevice::PickPhysicalDevice()
{
    auto                                 availablePhysicalDevices = m_instance.GetInstance().enumeratePhysicalDevices();
    std::array<DesiredDeviceFeatures, 2> desiredDeviceFeatures    = {
        GlobalVariables::GlobalStructs::primaryDeviceFeatures,
        GlobalVariables::GlobalStructs::secondaryDeviceFeatures,
    };
    for(int j = 0; j < desiredDeviceFeatures.size(); j++)
    {
        for(int i = 0; i < availablePhysicalDevices.size(); i++)
        {
            auto& physicalDevice = availablePhysicalDevices[i];
            Utils::Logger::LogInfoVerboseOnly("Found physical device: " + std::string(physicalDevice.getProperties().deviceName));
            if(desiredDeviceFeatures[j].CheckAgainstRetrievedPhysicalDevice(physicalDevice, m_instance.GetSurface()))
            {
                Utils::Logger::LogSuccess("Going to use: " + std::string(physicalDevice.getProperties().deviceName)
                                          + " device, terminating next searches");


                vk::PhysicalDeviceProperties2 physicalDeviceProperties;

                vk::PhysicalDeviceRayTracingPipelinePropertiesKHR    rayTracingPipelineProperties{};
                vk::PhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties{};
                accelerationStructureProperties.pNext = &rayTracingPipelineProperties;
                physicalDeviceProperties.pNext        = &accelerationStructureProperties;

                //GlobalVariables::GlobalStructs::GpuProperties.pNext = &rayTracingPipelineProperties;

                physicalDevice.getProperties2(&physicalDeviceProperties);
                GlobalVariables::GlobalStructs::GpuMemoryProperties = physicalDevice.getMemoryProperties2();

                GlobalVariables::GlobalStructs::GpuProperties                = physicalDeviceProperties;
                GlobalVariables::GlobalStructs::RayTracingPipelineProperties = rayTracingPipelineProperties;
                GlobalVariables::GlobalStructs::AccelerationStructProperties = accelerationStructureProperties;

                return physicalDevice;
            }
        }
    }
    // for the wors case allways return CPU
    for(auto& device : availablePhysicalDevices)
    {
        if(device.getProperties().deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
        {
            Utils::Logger::LogError("Forcing the use of CPU !");
            Utils::Logger::LogSuccess("Going to use: " + std::string(device.getProperties().deviceName));

            GlobalVariables::GlobalStructs::GpuProperties       = device.getProperties();
            GlobalVariables::GlobalStructs::GpuMemoryProperties = device.getMemoryProperties();
            return device;
        }
    }
    throw std::runtime_error("No suitable GPU found!");
}

VulkanCore::VDevice::VDevice(const VulkanCore::VulkanInstance& instance)
    : m_instance(instance)
{
    m_physicalDevice = PickPhysicalDevice();
    CreateLogicalDevice();
    CreateVmaAllocator(instance);
    FetchMaxSampleCount();

    for(int i = 0; i < m_transferCommandPool.size(); i++)
    {
        m_transferCommandPool[i] = std::make_unique<VulkanCore::VCommandPool>(*this, Transfer);
    }
    m_transferCommandPoolForSingleThread = std::make_unique<VulkanCore::VCommandPool>(*this, Transfer);
    DispatchLoader = vk::detail::DispatchLoaderDynamic(m_instance.GetInstance(), vkGetInstanceProcAddr);
    RetreiveDepthFormat();

    m_transferOpsManager = std::make_unique<VulkanUtils::VTransferOperationsManager>(*this);
    m_transferOpsManager->StartRecording();
    m_meshDataManager    = std::make_unique<MeshDatatManager>(*this);
    m_descriptorAllocator = std::make_unique<VulkanCore::VDescriptorAllocator>(*this);
}

VulkanCore::VCommandPool& VulkanCore::VDevice::GetTransferCommandPool() const
{
    // loop through every command pool and return the first one that is not in use
    for(int i = 0; i < m_transferCommandPool.size(); i++)
    {
        if(!m_transferCommandPool[i]->IsInUse())
        {
            m_transferCommandPool[i]->SetInUse(true);
            return *m_transferCommandPool[i];
        }
    }
}

VulkanCore::MeshDatatManager& VulkanCore::VDevice::GetMeshDataManager() const
{
    return *m_meshDataManager;
}

void VulkanCore::VDevice::CreateLogicalDevice()
{
    m_queueFamilyIndices = FindQueueFamilies(m_physicalDevice, m_instance);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t>                     uniqueQueueFamilies = {m_queueFamilyIndices.graphicsFamily.value().second,
                                                                  m_queueFamilyIndices.presentFamily.value().second,
                                                                  m_queueFamilyIndices.transferFamily.value().second,
                                                                  m_queueFamilyIndices.computeFamily.value().second};

    // we have to tell logical device what queue families it will have
    float queuePriority = 1.0f;
    for(auto queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }


    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.dynamicRendering = true;

    vk::PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT dynamicRenderingUnUsedAttachemnts;
    dynamicRenderingUnUsedAttachemnts.dynamicRenderingUnusedAttachments = true;
    dynamicRenderingUnUsedAttachemnts.pNext                             = &dynamicRenderingFeatures;

    vk::PhysicalDeviceVulkan12Features physicalDeviceVulkan12Features;
    physicalDeviceVulkan12Features.timelineSemaphore   = true;
    physicalDeviceVulkan12Features.bufferDeviceAddress = true;
    physicalDeviceVulkan12Features.hostQueryReset = true;

    //descriptor indexing feature
    physicalDeviceVulkan12Features.descriptorIndexing = true;
    physicalDeviceVulkan12Features.shaderSampledImageArrayNonUniformIndexing = true;
    physicalDeviceVulkan12Features.runtimeDescriptorArray = true;
    physicalDeviceVulkan12Features.descriptorBindingVariableDescriptorCount = true;
    physicalDeviceVulkan12Features.descriptorBindingPartiallyBound = true;
    physicalDeviceVulkan12Features.descriptorBindingUpdateUnusedWhilePending = true;
    physicalDeviceVulkan12Features.descriptorBindingSampledImageUpdateAfterBind = true;
    physicalDeviceVulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = true;
    physicalDeviceVulkan12Features.descriptorBindingUniformBufferUpdateAfterBind = true;


    // used in fore frame captures....
    if(GlobalState::ValidationLayersEnabled)
    {
        physicalDeviceVulkan12Features.bufferDeviceAddressCaptureReplay = true;
    }

    vk::PhysicalDeviceVulkan11Features oneOneFeatures;

    oneOneFeatures.shaderDrawParameters = true;
    oneOneFeatures.pNext = &dynamicRenderingUnUsedAttachemnts;

    physicalDeviceVulkan12Features.pNext = oneOneFeatures;

    vk::PhysicalDeviceAccelerationStructureFeaturesKHR GpuAccelerationStrucutreFeatures = {};
    GpuAccelerationStrucutreFeatures.accelerationStructure                              = true;
    GpuAccelerationStrucutreFeatures.pNext = &physicalDeviceVulkan12Features;

    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {};
    rayTracingPipelineFeatures.rayTracingPipeline                              = true;
    rayTracingPipelineFeatures.pNext                                           = &GpuAccelerationStrucutreFeatures;


    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid  = true;
    deviceFeatures.samplerAnisotropy = true;
    deviceFeatures.wideLines         = true;


    //create the logical device
    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;
    deviceCreateInfo.ppEnabledExtensionNames = GlobalVariables::deviceLevelExtensions.data();
    for(auto deviceExtension : GlobalVariables::deviceLevelExtensions)
    {
        Utils::Logger::LogInfo("Going to use extension:\t" + std::string(deviceExtension));
    }

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(GlobalVariables::deviceLevelExtensions.size());

    if(GlobalState::ValidationLayersEnabled)
    {
        deviceCreateInfo.enabledLayerCount   = GlobalVariables::validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = GlobalVariables::validationLayers.data();
    }
    else
    {
        deviceCreateInfo.enabledLayerCount = 0;
    }
    deviceCreateInfo.pNext = &rayTracingPipelineFeatures;

    m_device = m_physicalDevice.createDevice(deviceCreateInfo);
    assert(m_device);
    Utils::Logger::LogSuccess("Successfully created logical device");

    m_graphicsQueue = m_device.getQueue(m_queueFamilyIndices.graphicsFamily.value().second, 0);
    assert(m_graphicsQueue != VK_NULL_HANDLE);
    Utils::Logger::LogSuccess("Successfully retrieved graphics queue");

    m_presentQueue = m_device.getQueue(m_queueFamilyIndices.presentFamily.value().second, 0);
    assert(m_presentQueue != VK_NULL_HANDLE);
    Utils::Logger::LogSuccess("Successfully retrieved present queue");

    m_transferQueue = m_device.getQueue(m_queueFamilyIndices.transferFamily.value().second, 0);
    assert(m_transferQueue != VK_NULL_HANDLE);
    Utils::Logger::LogSuccess("Successfully retrieved transfer queue");

    m_computeQueue = m_device.getQueue(m_queueFamilyIndices.computeFamily.value().second, 0);
    assert(m_computeQueue != VK_NULL_HANDLE);
    Utils::Logger::LogSuccess("Successfully retrieved compute queue");
}

void VulkanCore::VDevice::CreateVmaAllocator(const VulkanCore::VulkanInstance& instance)
{
    Utils::Logger::LogInfoVerboseOnly("Creating VMA allocator");
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice         = m_physicalDevice;
    allocatorInfo.device                 = m_device;
    allocatorInfo.instance               = instance.GetInstance();
    allocatorInfo.flags                  = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    assert(vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator) == VK_SUCCESS);
    Utils::Logger::LogSuccess("Successfully created Vulkan Memory Allocator instance");
}

void VulkanCore::VDevice::RetreiveDepthFormat()
{
    std::vector<vk::Format> formats = {vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint};
    auto tilling = vk::ImageTiling::eOptimal;
    auto feature = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

    for(auto& format : formats)
    {
        vk::FormatProperties props;
        m_physicalDevice.getFormatProperties(format, &props);

        if(tilling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & feature) == feature)
        {
            m_depthFormat = format;
            return;
            ;
        }
        else if(tilling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & feature) == feature)
        {
            m_depthFormat = format;
            return;
        }
    }

    throw std::runtime_error("Depth format does is not supported on this device ");
}

void VulkanCore::VDevice::FetchMaxSampleCount()
{
    auto& gpuProperteis = GlobalVariables::GlobalStructs::GpuProperties;

    vk::SampleCountFlags maxSampleCount = gpuProperteis.properties.limits.framebufferColorSampleCounts
                                          & gpuProperteis.properties.limits.framebufferDepthSampleCounts;
    if(!GlobalState::MSAA)
    {
        m_sampleCount = vk::SampleCountFlagBits::e1;
        return;
    }

    vk::SampleCountFlagBits sampleCount = VulkanUtils::IntToVkSample(GlobalVariables::RenderingOptions::MSAASamples);

    if((maxSampleCount & sampleCount) == sampleCount)
    {
        m_sampleCount = sampleCount;
        return;
    }

    if(maxSampleCount & vk::SampleCountFlagBits::e64)
    {
        m_sampleCount = vk::SampleCountFlagBits::e64;
        return;
    }
    if(maxSampleCount & vk::SampleCountFlagBits::e32)
    {
        m_sampleCount = vk::SampleCountFlagBits::e32;
        return;
    }
    if(maxSampleCount & vk::SampleCountFlagBits::e16)
    {
        m_sampleCount = vk::SampleCountFlagBits::e16;
        return;
    }
    if(maxSampleCount & vk::SampleCountFlagBits::e8)
    {
        m_sampleCount = vk::SampleCountFlagBits::e8;
        return;
    }
    if(maxSampleCount & vk::SampleCountFlagBits::e4)
    {
        m_sampleCount = vk::SampleCountFlagBits::e4;
        return;
    }
    if(maxSampleCount & vk::SampleCountFlagBits::e2)
    {
        m_sampleCount = vk::SampleCountFlagBits::e2;
        return;
    }

    //default
    m_sampleCount = vk::SampleCountFlagBits::e1;
}

const uint32_t& VulkanCore::VDevice::GetConcreteQueueFamilyIndex(EQueueFamilyIndexType queueFamilyType) const
{
    assert(m_queueFamilyIndices.isComplete());
    switch(queueFamilyType)
    {
        case EQueueFamilyIndexType::Graphics:
            return m_queueFamilyIndices.graphicsFamily.value().second;
        case EQueueFamilyIndexType::PresentKHR:
            return m_queueFamilyIndices.presentFamily.value().second;
        case EQueueFamilyIndexType::Transfer:
            return m_queueFamilyIndices.transferFamily.value().second;
        case EQueueFamilyIndexType::Compute:
            return m_queueFamilyIndices.computeFamily.value().second;
        default:
            throw std::runtime_error("Invalid queue family index");
    }
}

const std::string VulkanCore::VDevice::GetQueueFamilyString(EQueueFamilyIndexType queueFamilyType) const
{
    assert(m_queueFamilyIndices.isComplete());
    switch(queueFamilyType)
    {
        case EQueueFamilyIndexType::Graphics:
            return "GRAPHICS";
        case EQueueFamilyIndexType::PresentKHR:
            return "PRESENT";
        case EQueueFamilyIndexType::Transfer:
            return "TRANSFER";
        case EQueueFamilyIndexType::Compute:
            return "COMPUTE";
        default:
            throw std::runtime_error("Invalid queue family index");
    }
}

void VulkanCore::VDevice::UpdateMemoryStatistics()
{
    vmaCalculateStatistics(m_vmaAllocator, &m_vmaStatistics);
}


void VulkanCore::VDevice::Destroy()
{
    m_meshDataManager->Destroy();
    m_transferOpsManager->Destroy();
    for(int i = 0; i < m_transferCommandPool.size(); i++)
    {
        m_transferCommandPool[i]->Destroy();
    }
    m_transferCommandPoolForSingleThread->Destroy();
    vmaDestroyAllocator(m_vmaAllocator);
    m_device.destroy();
    Utils::Logger::LogInfoVerboseOnly("Logical device destroyed");
}
