//
// Created by wpsimon09 on 30/09/24.
//

#include "VShader.hpp"

#include <fstream>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanCore {

void ReflecSetLayoutData::Print() const {
    std::cout<<"-- Bindings in set -:\n";
    for (auto& binding: variableNames) {
        std::cout<<"\t ";
        std::cout<<binding.first<<"\n";
    }
}
void ReflectionData::Print() const  {
    std::cout<<"======================= Reflection data ============================\n";

    for (int i_set = 0; i_set<descriptorSets.size(); i_set++) {
        std::cout<<"- Set" + std::to_string(i_set)<<" -- \n";
        descriptorSets.at(i_set).Print();
        std::cout<<"====================================================================\n\n";

    }
}
//=============================================
// REFLECTION DATA
//=============================================
void ReflectionData::Init(const void* byteCode, size_t size) {}

void ReflectionData::AddShader(const void* byteCode, size_t size, vk::ShaderStageFlags stage)
{
    auto result = spvReflectCreateShaderModule(size, byteCode, &moduleReflection);
    assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to reflect shader bytecode ensure size and code is correct ");

    //get how many set layouts are in the shader
    uint32_t count = 0;
    result         = spvReflectEnumerateDescriptorSets(&moduleReflection, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to enumerate descriptor set counts ");

    // get the concrete sets
    std::vector<SpvReflectDescriptorSet*> sets(count);
    result = spvReflectEnumerateDescriptorSets(&moduleReflection, &count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to retrieve binding handles ");

    // go through each descriptor set
    for(size_t i_set = 0; i_set < sets.size(); i_set++)
    {

        ReflecSetLayoutData newBindings;
        newBindings.bindings.resize(sets.size());
        // go through each binding in set the set
        const SpvReflectDescriptorSet& reflSet = *(sets[i_set]);
        newBindings.bindings.reserve(reflSet.binding_count);
        newBindings.variableNames.resize(reflSet.binding_count);
        descriptorSets[i_set].descriptorFlags.resize(reflSet.binding_count);


        for(uint32_t i_binding = 0; i_binding < reflSet.binding_count; i_binding++)
        {
            const SpvReflectDescriptorBinding reflBinding = *(reflSet.bindings[i_binding]);
            vk::DescriptorSetLayoutBinding    binding;
            binding.binding         = reflBinding.binding;
            binding.descriptorType  = static_cast<vk::DescriptorType>(reflBinding.descriptor_type);
            binding.descriptorCount = 1;
            for(uint32_t i_dim = 0; i_dim < reflBinding.array.dims_count; ++i_dim)
            {
                binding.descriptorCount *= reflBinding.array.dims[i_dim];
            }

            binding.stageFlags = vk::ShaderStageFlagBits::eAll;

            newBindings.bindings.push_back( binding);
            newBindings.variableNames.push_back({std::to_string(binding.binding) + ": " + reflBinding.name, binding.descriptorType});
            //newBindings.shaderStages[i_binding] = {std::to_string(i_binding) + ": " + reflBinding.name, binding.descriptorType};
        }

        // set the descriptor set layout
        descriptorSets[i_set].setNumber = i_set;

        // take the current bindings from the descriptor set
        auto& currentBindings = descriptorSets[i_set];

        // insert currently cerated bindings to the newly ones
        currentBindings.bindings.insert(currentBindings.bindings.end(), newBindings.bindings.begin(), newBindings.bindings.end());
        currentBindings.variableNames.insert(currentBindings.variableNames.end(), newBindings.variableNames.begin(), newBindings.variableNames.end());

        currentBindings.createInfo.bindingCount = currentBindings.bindings.size();

        //=============================================
        // vk::DescriptorSetLayoutBindingFlagsCreateInfo
        currentBindings.descriptorFlags = std::vector<vk::DescriptorBindingFlags>(currentBindings.bindings.size(),
        vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::ePartiallyBound);

        currentBindings.bindingFlagsInfo.bindingCount = currentBindings.descriptorFlags.size();
        currentBindings.bindingFlagsInfo.pBindingFlags = currentBindings.descriptorFlags.data();

        //============================================
        // vk::DescriptorSetLayoutCreateInfo
        currentBindings.createInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
        currentBindings.createInfo.pNext = &descriptorSets[i_set].bindingFlagsInfo;
;


        descriptorSets[i_set].createInfo.pBindings = currentBindings.bindings.data();
    }
}


void ReflectionData::Destroy()
{
    spvReflectDestroyShaderModule(&moduleReflection);
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

const ReflectionData& VShader::GetReflectionData() const
{
    return m_shaderReeflection;
}

void VShader::CreateShaderModules()
{

    Utils::Logger::LogInfoVerboseOnly("Creating shader modules...");

    auto vertexSPRIV   = VulkanUtils::ReadSPIRVShader(m_vertexSource);
    auto fragmentSPRIV = VulkanUtils::ReadSPIRVShader(m_fragmentSource);

    m_vertexShaderModule   = VulkanUtils::CreateShaderModule(m_device, vertexSPRIV);
    m_fragmentShaderModule = VulkanUtils::CreateShaderModule(m_device, fragmentSPRIV);

    //SpvReflectResult result = spvReflectCreateShaderModule(vertexSPRIV.size() * sizeof(char),vertexSPRIV.data(), &m_vertexReflection) ;

    m_shaderReeflection.AddShader(vertexSPRIV.data(), sizeof(char) * vertexSPRIV.size(), vk::ShaderStageFlagBits::eVertex);
    m_shaderReeflection.AddShader(fragmentSPRIV.data(), sizeof(char) * fragmentSPRIV.size(), vk::ShaderStageFlagBits::eFragment);

    // assert(result == SPV_REFLECT_RESULT_SUCCESS);

    uint32_t varCount = 0;
    // result = spvReflectEnumerateInputVariables(&m_vertexReflection, &varCount, nullptr);

    // assert(result == SPV_REFLECT_RESULT_SUCCESS);


    if(m_computeSource.has_value())
    {
        auto computeSPRIV     = VulkanUtils::ReadSPIRVShader(m_computeSource.value());
        m_computeShaderModule = VulkanUtils::CreateShaderModule(m_device, computeSPRIV);
        m_computeReflection->Init(m_computeSource.value().data(), computeSPRIV.size() * sizeof(char));
    }
    else
        Utils::Logger::LogInfoVerboseOnly("Compute shader was not specified");

    Utils::Logger::LogSuccess("Shader modules created !");
}
}  // namespace VulkanCore