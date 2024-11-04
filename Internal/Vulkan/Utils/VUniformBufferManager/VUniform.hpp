//
// Created by wpsimon09 on 03/11/24.
//

#ifndef VUNIFROM_HPP
#define VUNIFROM_HPP
#include <memory>

#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

namespace VulkanUtils {

template<typename T>
class VUniform {
public:
    explicit VUniform(const VulkanCore::VDevice& device);
    const T& GetUBOStruct(){return *m_uniformCPU;};
    void UpdateGPUBuffer(int frameIndex);
    const std::vector<vk::DescriptorBufferInfo>& GetDescriptorBufferInfos() const {return m_bufferInfo;};
    void Destory() const;
private:
    std::unique_ptr<T> m_uniformCPU;
    std::vector<std::unique_ptr<VulkanCore::VBuffer>> m_uniformGPU;
    std::vector<vk::DescriptorBufferInfo> m_bufferInfo;
};

template <typename T>
VUniform<T>::VUniform(const VulkanCore::VDevice& device) {
    m_uniformCPU = std::make_unique<T>();
    m_uniformGPU.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_bufferInfo.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
        m_uniformGPU[i] = std::make_unique<VulkanCore::VBuffer>(device);
        m_uniformGPU[i]->MakeUniformBuffer(m_uniformCPU.get());
        m_bufferInfo[i] = m_uniformGPU[i]->GetBufferInfoForDescriptor();
    }
}

template <typename T>
void VUniform<T>::UpdateGPUBuffer(int frameIndex) {
    memcpy(m_uniformGPU[frameIndex]->GetMapPointer(), m_uniformCPU.get(), sizeof(T));
}

template <typename T>
void VUniform<T>::Destory() const {
    for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
        m_uniformGPU[i]->Destroy();
    }
}

} // VulkanUtils

#endif //VUNIFROM_HPP
