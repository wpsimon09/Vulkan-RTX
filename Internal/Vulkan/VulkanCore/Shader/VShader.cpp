//
// Created by wpsimon09 on 30/09/24.
//

#include "VShader.hpp"

#include <fstream>
#include <vulkan/vulkan_enums.hpp>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanCore {

void ReflecSetLayoutData::Print() const
{
    std::cout << "-- Bindings in set -:\n";
    int i_binding = 0;
    for(auto& binding : variableNames)
    {
        std::cout << "\t ";

        if(bindings[i_binding].descriptorCount > 1)
        {
            std::cout << binding.first + " := type =: " + VulkanUtils::DescriptorTypeToString(binding.second) + " [ "
                             + std::to_string(bindings[i_binding].descriptorCount) + "]\n";
        }
        else
        {
            std::cout << binding.first << +" := type =: " + VulkanUtils::DescriptorTypeToString(binding.second) + "\n";
        }


        i_binding++;
    }
}
void ReflectionData::Print() const
{
    std::cout << "======================= Reflection data ============================\n";

    if(!pushConstantNames.empty())
    {
        for(int i = 0; i < pushConstantNames.size(); i++)
        {
            std::cout << "Push constant: " + pushConstantNames[i] + "size: " + std::to_string(this->PCs[i].size) << "\n";
        }
    }
    for(int i_set = 0; i_set < descriptorSets.size(); i_set++)
    {
        std::cout << "- Set" + std::to_string(i_set) << " -- \n";
        descriptorSets.at(i_set).Print();
        std::cout << "--------------------------------------------------------------\n\n";
    }

    std::cout << "====================================================================\n";
}
//=============================================
// REFLECTION DATA
//=============================================
void ReflectionData::Init(const void* byteCode, size_t size) {}

void ReflectionData::AddShader(const void* byteCode, size_t size, vk::ShaderStageFlags stage, vk::DescriptorBindingFlags decriptorFlags)
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

    //=================================
    // push constants
    uint32_t pcCount = 0;
    result           = spvReflectEnumeratePushConstantBlocks(&moduleReflection, &pcCount, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to enumerate push constant blocks ");

    std::vector<SpvReflectBlockVariable*> pushConstants;
    if(pcCount > 0)
    {
        pushConstants.resize(pcCount);
        result = spvReflectEnumeratePushConstantBlocks(&moduleReflection, &pcCount, pushConstants.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to retrieve push constant blocks ");

        PCs.resize(pcCount);
        pushConstantNames.resize(pcCount);
        for(int i = 0; i < pcCount; i++)
        {

            pushConstantNames[i] = pushConstants[i]->name;
            PCs[i].offset        = pushConstants[i]->offset;
            PCs[i].size          = pushConstants[i]->size;

            PCs[i].stageFlags = vk::ShaderStageFlagBits::eAll;
        }
    }


    //descriptorSets.reserve(sets.size());

    //=================================
    // go through each descriptor set
    for(auto& set : sets)
    {

        ReflecSetLayoutData newSetLayout;

        uint32_t setIndex = set->set;

        //============================================
        // get binding information
        const SpvReflectDescriptorSet& reflSet = *(set);
        newSetLayout.bindings.reserve(reflSet.binding_count);
        newSetLayout.variableNames.reserve(reflSet.binding_count);
        descriptorSets[setIndex].descriptorFlags.resize(reflSet.binding_count);

        //===========================================
        // go through each binding in descriptor set
        for(uint32_t i_binding = 0; i_binding < reflSet.binding_count; i_binding++)
        {
            const SpvReflectDescriptorBinding reflBinding = *(reflSet.bindings[i_binding]);
            vk::DescriptorSetLayoutBinding    binding;
            binding.binding        = reflBinding.binding;
            binding.descriptorType = static_cast<vk::DescriptorType>(reflBinding.descriptor_type);

            binding.descriptorCount = 1;

            //================================
            // gets dimmensions of the array
            auto dims = reflBinding.array.dims_count;

            //================================
            // if dimensions are 0 this means we have array with arbitary size
            // e.g. Sampler2D textures[];
            if(dims == 1 && reflBinding.array.dims[0] == 0)
            {
                switch(binding.descriptorType)
                {

                    case vk::DescriptorType::eCombinedImageSampler:
                        binding.descriptorCount = 1000;
                        break;
                    default:
                        binding.descriptorCount = 1;
                        break;
                }
            }
            else
            {
                for(uint32_t i_dim = 0; i_dim < dims; ++i_dim)
                    binding.descriptorCount *= reflBinding.array.dims[i_dim];
            }


            assert(binding.descriptorCount > 0 && "descriptor count must be 0 ");

            binding.stageFlags = vk::ShaderStageFlagBits::eAll;


            newSetLayout.bindings.emplace_back(binding);
            newSetLayout.variableNames.emplace_back(std::to_string(binding.binding) + ": " + reflBinding.name, binding.descriptorType);
        }

        // set the descriptor set layout
        descriptorSets[setIndex].setNumber = setIndex;

        // take the current bindings from the descriptor set
        auto& currentSet = descriptorSets[setIndex];

        // insert currently cerated bindings to the newly ones
        currentSet.bindings.insert(currentSet.bindings.end(), newSetLayout.bindings.begin(), newSetLayout.bindings.end());

        currentSet.variableNames.insert(currentSet.variableNames.end(), newSetLayout.variableNames.begin(),
                                        newSetLayout.variableNames.end());


        //==============================================
        // vk::DescriptorSetLayoutBindingFlagsCreateInfo
        currentSet.descriptorFlags.resize(currentSet.bindings.size());
        int i = 0;
        for(auto& bindingInSet : currentSet.bindings)
        {
            // special prefix for the descriptors in shaders will allow for them to be "update after bind (uab)"
            bool isUpdateAfterBind = (currentSet.variableNames[i].first.substr(0, 4) == "uab");

            if(isUpdateAfterBind)
            {
                currentSet.descriptorFlags[i] |= vk::DescriptorBindingFlagBits::eUpdateAfterBind;
            }
            else
            {
                currentSet.descriptorFlags[i] = {};
            }
            i++;
        }

        currentSet.bindingFlagsInfo.bindingCount  = currentSet.descriptorFlags.size();
        currentSet.bindingFlagsInfo.pBindingFlags = currentSet.descriptorFlags.data();

        //============================================
        // vk::DescriptorSetLayoutCreateInfo
        currentSet.createInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
        currentSet.createInfo.pNext = &descriptorSets[setIndex].bindingFlagsInfo;
        ;


        currentSet.createInfo.bindingCount = currentSet.bindings.size();
        currentSet.createInfo.pBindings    = currentSet.bindings.data();
    }
}


void ReflectionData::Destroy()
{
    spvReflectDestroyShaderModule(&moduleReflection);
}

//=============================================
// SHADER ABSTRACTION
//=============================================
VShader::VShader(const VulkanCore::VDevice& device, const std::string& vertexSource, const std::string& fragmentSource, vk::DescriptorBindingFlags bindnigFlags)
    : m_device(device)
{
    m_vertexSource   = vertexSource;
    m_fragmentSource = fragmentSource;

    CreateShaderModules();
}

VShader::VShader(const VulkanCore::VDevice& device, const std::string& computeShaderSource, vk::DescriptorBindingFlags bindnigFlags)
    : m_device(device)
{
    m_vertexSource   = "";
    m_fragmentSource = "";
    m_computeSource  = computeShaderSource;

    auto computeSPRIV     = VulkanUtils::ReadSPIRVShader(m_computeSource.value());
    m_computeShaderModule = VulkanUtils::CreateShaderModule(m_device, computeSPRIV);
    m_shaderReeflection.AddShader(computeSPRIV.data(), computeSPRIV.size() * sizeof(char), vk::ShaderStageFlagBits::eAll, bindnigFlags);
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

void VShader::CreateShaderModules(vk::DescriptorBindingFlags bindingFlags)
{

    Utils::Logger::LogInfoVerboseOnly("Creating shader modules...");

    auto vertexSPRIV   = VulkanUtils::ReadSPIRVShader(m_vertexSource);
    auto fragmentSPRIV = VulkanUtils::ReadSPIRVShader(m_fragmentSource);

    m_vertexShaderModule   = VulkanUtils::CreateShaderModule(m_device, vertexSPRIV);
    m_fragmentShaderModule = VulkanUtils::CreateShaderModule(m_device, fragmentSPRIV);

    //SpvReflectResult result = spvReflectCreateShaderModule(vertexSPRIV.size() * sizeof(char),vertexSPRIV.data(), &m_vertexReflection) ;

    m_shaderReeflection.AddShader(vertexSPRIV.data(), sizeof(char) * vertexSPRIV.size(), vk::ShaderStageFlagBits::eVertex, bindingFlags);
    m_shaderReeflection.AddShader(fragmentSPRIV.data(), sizeof(char) * fragmentSPRIV.size(),
                                  vk::ShaderStageFlagBits::eFragment, bindingFlags);

    // assert(result == SPV_REFLECT_RESULT_SUCCESS);

    uint32_t varCount = 0;
    // result = spvReflectEnumerateInputVariables(&m_vertexReflection, &varCount, nullptr);

    // assert(result == SPV_REFLECT_RESULT_SUCCESS);
    Utils::Logger::LogSuccess("Shader modules created !");
}
}  // namespace VulkanCore