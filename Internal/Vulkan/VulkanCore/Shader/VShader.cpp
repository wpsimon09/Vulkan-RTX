//
// Created by wpsimon09 on 30/09/24.
//

#include "VShader.hpp"

#include <fstream>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanCore {

//=============================================
// REFLECTION DATA
//=============================================
void ReflectionData::Init(const void* byteCode, size_t size)
{
    auto result = spvReflectCreateShaderModule(size, byteCode, &m_moduleReflection);
    assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to reflect shader bytecode ensure size and code is correct ");

    //get how many set layouts are in the shader
    uint32_t count = 0;
    result = spvReflectEnumerateDescriptorSets(&m_moduleReflection, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to enumerate descriptor set counts ");

    // get the concrete sets
    std::vector<SpvReflectDescriptorSet*> sets(count);
    result = spvReflectEnumerateDescriptorSets(&m_moduleReflection, &count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to retrieve binding handles ");

    setLayouts.reserve(sets.size());
    // go through each descriptor set
    for (size_t i_set = 0; i_set < sets.size(); i_set++) {
        // go through each binding in set the set
        const SpvReflectDescriptorSet& reflSet = *(sets[i_set]);
        setLayouts[i_set].bindings.resize(reflSet.binding_count);
        for (uint32_t i_binding = 0; i_binding < reflSet.binding_count; i_binding++) {
            //const  init binding
        }
    }
}
void ReflectionData::Destroy() {
    spvReflectDestroyShaderModule(&m_moduleReflection);
}

//=============================================
// SHADER ABSTRACTION
//=============================================
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

    //SpvReflectResult result = spvReflectCreateShaderModule(vertexSPRIV.size() * sizeof(char),vertexSPRIV.data(), &m_vertexReflection) ;

    m_vertexReflection.Init(vertexSPRIV.data(), sizeof(char) * vertexSPRIV.size());
    m_fragmentReflection.Init(fragmentSPRIV.data(), sizeof(char) * fragmentSPRIV.size());

   // assert(result == SPV_REFLECT_RESULT_SUCCESS);

    uint32_t varCount = 0;
   // result = spvReflectEnumerateInputVariables(&m_vertexReflection, &varCount, nullptr);

   // assert(result == SPV_REFLECT_RESULT_SUCCESS);


    if(m_computeSource.has_value())
    {
        auto                       computeSPRIV = VulkanUtils::ReadSPIRVShader(m_computeSource.value());
        m_computeShaderModule = VulkanUtils::CreateShaderModule(m_device, computeSPRIV);
        m_computeReflection->Init(m_computeSource.value().data(), computeSPRIV.size() * sizeof(char));
    }
    else
        Utils::Logger::LogInfoVerboseOnly("Compute shader was not specified");

    Utils::Logger::LogSuccess("Shader modules created !");
}
}  // namespace VulkanCore