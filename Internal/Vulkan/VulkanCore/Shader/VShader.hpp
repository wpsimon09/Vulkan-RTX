//
// Created by wpsimon09 on 30/09/24.
//

#ifndef VSHADER_HPP
#define VSHADER_HPP
#include <string>
#include <vulkan/vulkan.hpp>

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "SPIRV-Reflect/spirv_reflect.h"

namespace VulkanCore {

struct ReflecSetLayoutData {
  uint32_t setNumber;
  vk::DescriptorSetLayoutCreateInfo createInfo;
  // used to store descriptor flags (like : update after bind_bit, allow partially bound etc...)
  vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo ;

  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  std::vector<std::pair<std::string, SpvReflectShaderStageFlagBits>> shaderStages;
  std::vector<std::pair<std::string, vk::DescriptorType>> variableNames;

  std::vector<vk::DescriptorBindingFlags> descriptorFlags;


  void Print() const;

};

struct ReflectionData {
  SpvReflectShaderModule moduleReflection;
  std::unordered_map<int , ReflecSetLayoutData> descriptorSets;
  std::vector<vk::PushConstantRange> PCs;
  std::string pushConstantName;

  int bindingCount = 0;


  void Print() const ;
  void Init(const void* byteCode, size_t size);
  void AddShader(const void* byteCode, size_t size, vk::ShaderStageFlags stage);
  void Destroy();

};

class VShader : public VObject
{

  public:
    VShader(const VulkanCore::VDevice& device,
            const std::string&         vertexSource,
            const std::string&         fragmentSource,
            const std::string&         computeSource = "");
    void                    DestroyExistingShaderModules();
    const vk::ShaderModule& GetShaderModule(GlobalVariables::SHADER_TYPE shaderType) const;
    const ReflectionData&   GetReflectionData() const;

  private:
    void CreateShaderModules();

  private:
    const VulkanCore::VDevice& m_device;

    ReflectionData                m_fragmentReflection;
    ReflectionData                m_vertexReflection;

    ReflectionData                m_shaderReeflection;



    std::optional<ReflectionData> m_computeReflection;

    vk::ShaderModule                m_fragmentShaderModule;
    vk::ShaderModule                m_vertexShaderModule;
    std::optional<vk::ShaderModule> m_computeShaderModule;

    std::string                m_vertexSource;
    std::string                m_fragmentSource;
    std::optional<std::string> m_computeSource;
};

}  // namespace VulkanCore

#endif  //VSHADER_HPP
