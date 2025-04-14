//
// Created by wpsimon09 on 30/09/24.
//

#include "VShader.hpp"

#include <fstream>

#include "Application/Logger/Logger.hpp"

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

std::vector<char> VShader::ReadSPIRVShader(const std::string& SPIRVShader)
{
    std::ifstream file(SPIRVShader, std::ios::ate | std::ios::binary);

    if(!file.is_open())
    {
        const auto err = "Failed to open SPIRV shader file at path: " + SPIRVShader + " did you compile the shaders using compile.sh script ?";
        throw std::runtime_error(err);
    }

    //create buffer to hold the binary
    size_t            fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    //go back to the begining and read the file again to get the content
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    assert(buffer.size() == fileSize);
    return buffer;
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

    auto vertexSPRIV   = ReadSPIRVShader(m_vertexSource);
    auto fragmentSPRIV = ReadSPIRVShader(m_fragmentSource);

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
}  // namespace VulkanCore