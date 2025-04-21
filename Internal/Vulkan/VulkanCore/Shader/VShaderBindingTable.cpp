//
// Created by wpsimon09 on 21/04/25.
//

#include "VShaderBindingTable.hpp"

#include "VShader.hpp"

namespace VulkanCore {
VShaderBindingTable::VShaderBindingTable(const VulkanCore::VDevice& device, RTX::RTXShaderPaths& shaders):m_device(device) {
  CreateShaderModules(shaders);
}
void VShaderBindingTable::CreateShaderModules(RTX::RTXShaderPaths& shaders) {
  Utils::Logger::LogInfoVerboseOnly("Creating shader modules...");

  auto rayGenSPIRV = VShader::ReadSPIRVShader(shaders.rayGenPath);
  auto rayMissSPIRV   = VShader::ReadSPIRVShader(shaders.missPath);
  auto rayHITSPIRV   = VShader::ReadSPIRVShader(shaders.rayHitPath);

  vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo;
  vertexShaderModuleCreateInfo.codeSize = vertexSPRIV.size();
  vertexShaderModuleCreateInfo.pCode    = reinterpret_cast<uint32_t*>(vertexSPRIV.data());
  vertexShaderModuleCreateInfo.pNext    = nullptr;
  m_vertexShaderModule = m_device.GetDevice().createShaderModule(vertexShaderModuleCreateInfo, nullptr);
  assert(m_vertexShaderModule);
  Utils::Logger::LogInfoVerboseOnly("Created Vertex shader module");


  vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo;
  fragmentShaderModuleCreateInfo.codeSize = fragmentSPRIV.size();
  fragmentShaderModuleCreateInfo.pCode    = reinterpret_cast<uint32_t*>(fragmentSPRIV.data());
  fragmentShaderModuleCreateInfo.pNext    = nullptr;
  m_fragmentShaderModule = m_device.GetDevice().createShaderModule(fragmentShaderModuleCreateInfo, nullptr);
  assert(m_fragmentShaderModule);
  Utils::Logger::LogInfoVerboseOnly("Created Fragment shader module");


  if(m_computeSource.has_value())
  {
    auto                       computeSPRIV = ReadSPIRVShader(m_computeSource.value());
    vk::ShaderModuleCreateInfo computeShaderModuleCreateInfo;
    computeShaderModuleCreateInfo.codeSize = computeSPRIV.size();
    ;
    computeShaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(computeSPRIV.data());
    computeShaderModuleCreateInfo.pNext = nullptr;
    m_computeShaderModule = m_device.GetDevice().createShaderModule(computeShaderModuleCreateInfo, nullptr);
    assert(m_computeShaderModule.value());
    Utils::Logger::LogInfoVerboseOnly("Created Compute shader module");
  }
  else
    Utils::Logger::LogInfoVerboseOnly("Compute shader was not specified");

  Utils::Logger::LogSuccess("Shader modules created !");
}
} // VulkanCore