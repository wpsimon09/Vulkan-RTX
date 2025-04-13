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

template <typename T>
class VUniform
{
public:
  explicit VUniform(const VulkanCore::VDevice& device, bool oneTime = false);
  T&                                           GetUBOStruct() { return *m_uniformCPU; };
  void                                         UpdateGPUBuffer(int frameIndex);
  const std::vector<vk::DescriptorBufferInfo>& GetDescriptorBufferInfos() const { return m_bufferInfo; };
  void                                         Destory() const;
  bool                                         m_isDirty = true;

  ~VUniform();

private:
  int                                               m_framesInFlight;
  std::unique_ptr<T>                                m_uniformCPU;
  std::vector<std::unique_ptr<VulkanCore::VBuffer>> m_uniformGPU;  // per frame in flight
  std::vector<vk::DescriptorBufferInfo>             m_bufferInfo;
};

template <typename T>
VUniform<T>::VUniform(const VulkanCore::VDevice& device, bool oneTime)
{
  if(oneTime)
  {
    m_framesInFlight = 1;
  }
  else
  {
    m_framesInFlight = GlobalVariables::MAX_FRAMES_IN_FLIGHT;
  }
  m_uniformCPU = std::make_unique<T>();
  m_uniformGPU.resize(m_framesInFlight);
  m_bufferInfo.resize(m_framesInFlight);

  for(int i = 0; i < m_framesInFlight; i++)
  {
    m_uniformGPU[i] = std::make_unique<VulkanCore::VBuffer>(device, "<== UNIFORM BUFFER ==>");
    m_uniformGPU[i]->MakeUniformBuffer(m_uniformCPU.get(), sizeof(T));
    m_bufferInfo[i] = m_uniformGPU[i]->GetBufferInfoForDescriptor();
  }
}

template <typename T>
void VUniform<T>::UpdateGPUBuffer(int frameIndex)
{
  if(m_framesInFlight < 2)
  {
    frameIndex = 0;
  }
  memcpy(m_uniformGPU[frameIndex]->GetMapPointer(), m_uniformCPU.get(), sizeof(T));
  //m_isDirty = false; TODO: update data only if they are chagned
}

template <typename T>
void VUniform<T>::Destory() const
{
  for(int i = 0; i < m_framesInFlight; i++)
  {
    m_uniformGPU[i]->Destroy();
  }
}

template <typename T>
VUniform<T>::~VUniform()
{
}
}  // namespace VulkanUtils

#endif  //VUNIFROM_HPP
