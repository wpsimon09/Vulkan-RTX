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


namespace VulkanCore {

class VShader : public VObject
{
  public:
    VShader(const VulkanCore::VDevice& device,
            const std::string&         vertexSource,
            const std::string&         fragmentSource,
            const std::string&         computeSource = "");
    void                    DestroyExistingShaderModules();
    std::vector<char>       ReadSPIRVShader(const std::string& SPIRVShader);
    const vk::ShaderModule& GetShaderModule(GlobalVariables::SHADER_TYPE shaderType) const;

  private:
    void CreateShaderModules();

  private:
    const VulkanCore::VDevice& m_device;

    vk::ShaderModule                m_fragmentShaderModule;
    vk::ShaderModule                m_vertexShaderModule;
    std::optional<vk::ShaderModule> m_computeShaderModule;

    std::string                m_vertexSource;
    std::string                m_fragmentSource;
    std::optional<std::string> m_computeSource;
};

}  // namespace VulkanCore

#endif  //VSHADER_HPP
