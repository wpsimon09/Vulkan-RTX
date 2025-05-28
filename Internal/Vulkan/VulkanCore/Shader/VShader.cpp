//
// Created by wpsimon09 on 30/09/24.
//

#include "VShader.hpp"

#include <fstream>

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
    std::cout << "====================================================================\n";
    std::cout << "======================= Reflection data ============================\n";
    std::cout << "====================================================================\n";

    if (!pushConstantName.empty()) {
        std::cout << "Push constant: " +pushConstantName << "\n";
    }
    for(int i_set = 0; i_set < descriptorSets.size(); i_set++)
    {
        std::cout << "- Set" + std::to_string(i_set) << " -- \n";
        descriptorSets.at(i_set).Print();
        std::cout << "====================================================================\n\n";
    }

    std::cout << "====================================================================\n";
    std::cout << "====================================================================\n";
    std::cout << "====================================================================\n";

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

    //=================================
    // push constants
    uint32_t pcCount = 0;
    result = spvReflectEnumeratePushConstantBlocks(&moduleReflection, &pcCount, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to enumerate push constant blocks ");

    std::vector<SpvReflectBlockVariable*> pushConstants;
    if (pcCount > 0) {
        pushConstants.resize(pcCount);
        result = spvReflectEnumeratePushConstantBlocks(&moduleReflection, &pcCount, pushConstants.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS && "Failed to retrieve push constant blocks ");

        PCs.resize(pcCount);
        for (int i = 0; i < pcCount; i++) {

            pushConstantName = pushConstants[i]->name;
            PCs[i].offset = pushConstants[i]->offset;
            PCs[i].size   = pushConstants[i]->size;

            // TODO: for now I will only allow for push constants ot be in vertex shader
            PCs[i].stageFlags = vk::ShaderStageFlagBits::eVertex;
        }


    }


    //descriptorSets.reserve(sets.size());

    //=================================
    // go through each descriptor set
    for(auto& set: sets)
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
            binding.binding = reflBinding.binding;
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

            binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;


            newSetLayout.bindings.emplace_back(binding);
            newSetLayout.variableNames.emplace_back(std::to_string(binding.binding) + ": " + reflBinding.name, binding.descriptorType);
        }

        // set the descriptor set layout
        descriptorSets[setIndex].setNumber = setIndex;

        // take the current bindings from the descriptor set
        auto& currentBindings = descriptorSets[setIndex];

        // insert currently cerated bindings to the newly ones
        currentBindings.bindings.insert(currentBindings.bindings.end(), newSetLayout.bindings.begin(),
                                        newSetLayout.bindings.end());

        currentBindings.variableNames.insert(currentBindings.variableNames.end(), newSetLayout.variableNames.begin(),
                                             newSetLayout.variableNames.end());

        currentBindings.createInfo.bindingCount = currentBindings.bindings.size();

        //=============================================
        // vk::DescriptorSetLayoutBindingFlagsCreateInfo
        currentBindings.descriptorFlags = std::vector<vk::DescriptorBindingFlags>(
            currentBindings.bindings.size(), vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::ePartiallyBound
                                                 | vk::DescriptorBindingFlagBits::eVariableDescriptorCount);

        currentBindings.bindingFlagsInfo.bindingCount  = currentBindings.descriptorFlags.size();
        currentBindings.bindingFlagsInfo.pBindingFlags = currentBindings.descriptorFlags.data();

        //============================================
        // vk::DescriptorSetLayoutCreateInfo
        currentBindings.createInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
        currentBindings.createInfo.pNext = &descriptorSets[setIndex].bindingFlagsInfo;
        ;


        descriptorSets[setIndex].createInfo.pBindings = currentBindings.bindings.data();
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