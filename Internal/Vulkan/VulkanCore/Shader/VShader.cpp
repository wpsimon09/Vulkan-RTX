//
// Created by wpsimon09 on 30/09/24.
//

#include "VShader.hpp"

#include <fstream>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanCore {

VShader::VShader(const VulkanCore::VDevice& device, const std::string& vertexSource, const std::string& fragmentSource, const std::string& computeSource)
    : m_device(device)
{
    m_vertexSource   = vertexSource;
    m_fragmentSource = fragmentSource;
    if(computeSource != "")
    {
        m_computeSource = computeSource;
    }

    CreateShaderModules();
}

void VShader::DestroyExistingShaderModules()
{
    m_device.GetDevice().destroyShaderModule(m_vertexShaderModule);
    m_device.GetDevice().destroyShaderModule(m_fragmentShaderModule);
    if(m_computeShaderModule.has_value())
    {
        m_device.GetDevice().destroyShaderModule(m_computeShaderModule.value());
    }
    Utils::Logger::LogInfoVerboseOnly("Deleted all unnecessary shader modules");
}


const vk::ShaderModule& VShader::GetShaderModule(GlobalVariables::SHADER_TYPE shaderType) const
{
    switch(shaderType)
    {
        case GlobalVariables::SHADER_TYPE::VERTEX: {
            return m_vertexShaderModule;
        }
        case GlobalVariables::SHADER_TYPE::FRAGMENT: {
            return m_fragmentShaderModule;
        }
        case GlobalVariables::SHADER_TYPE::COMPUTE: {
            assert(m_computeShaderModule.has_value());
            return m_computeShaderModule.value();
            break;
        }
        default: {
            throw std::runtime_error("Unknown shader type");
            break;
        }
    }
}

void VShader::CreateShaderModules()
{

    Utils::Logger::LogInfoVerboseOnly("Creating shader modules...");

    auto vertexSPRIV   = VulkanUtils::ReadSPIRVShader(m_vertexSource);
    auto fragmentSPRIV = VulkanUtils::ReadSPIRVShader(m_fragmentSource);

    m_vertexShaderModule = VulkanUtils::CreateShaderModule(m_device, vertexSPRIV);
    m_fragmentShaderModule = VulkanUtils::CreateShaderModule(m_device, fragmentSPRIV);


    if(m_computeSource.has_value())
    {
        auto                       computeSPRIV = VulkanUtils::ReadSPIRVShader(m_computeSource.value());
        m_computeShaderModule = VulkanUtils::CreateShaderModule(m_device, computeSPRIV);
    }
    else
        Utils::Logger::LogInfoVerboseOnly("Compute shader was not specified");

    Utils::Logger::LogSuccess("Shader modules created !");
}
}  // namespace VulkanCore