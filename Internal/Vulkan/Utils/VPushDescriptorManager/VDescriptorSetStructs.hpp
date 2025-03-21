//
// Created by wpsimon09 on 18/03/25.
//

#ifndef VDESCRIPTORSETSTRUCTS_HPP
#define VDESCRIPTORSETSTRUCTS_HPP
#include <vulkan/vulkan.hpp>
namespace VulkanUtils
{
    enum class EDescriptorLayoutStruct: uint8_t
    {
        Basic = 0,
        UnlitSingleTexture,
        ForwardShading
    };

    struct BasicDescriptorSet
    {
        vk::DescriptorBufferInfo cameraUBOBuffer; // for camera uniform buffer
        vk::DescriptorBufferInfo meshUBBOBuffer; // for mesh uniform buffer
        vk::DescriptorBufferInfo extraBuffer;

        virtual ~BasicDescriptorSet() = default;
    };

    struct UnlitSingleTexture
    {
        vk::DescriptorBufferInfo cameraUBOBuffer; // for camera uniform buffer
        vk::DescriptorBufferInfo meshUBBOBuffer; // for mesh uniform buffer
        vk::DescriptorBufferInfo extraBuffer;

        vk::DescriptorImageInfo texture;
        // ohter look up tables, irradiance, radiance maps etc...
    };

    struct ForwardShadingDstSet
    {
        vk::DescriptorBufferInfo cameraUBOBuffer; // for camera uniform buffer
        vk::DescriptorBufferInfo meshUBBOBuffer; // for mesh uniform buffer
        vk::DescriptorBufferInfo extraBuffer;

        vk::DescriptorBufferInfo pbrMaterialNoTexture; // for material uniform buffer
        vk::DescriptorBufferInfo pbrMaterialFeatures; // for material uniform buffer
        vk::DescriptorBufferInfo lightInformation; // all light information

        vk::DescriptorImageInfo diffuseTextureImage; // for diffuse
        vk::DescriptorImageInfo normalTextureImage; // for normal
        vk::DescriptorImageInfo armTextureImage; // for arm
        vk::DescriptorImageInfo emissiveTextureImage; // for emisive texture

        vk::DescriptorImageInfo LUT_LTC; // for linearly transformed cosines
        vk::DescriptorImageInfo LUT_LTC_Inverse; // for linearly transformed cosines

        // ohter look up tables, irradiance, radiance maps etc...
    };

    using DescriptorSetTemplateVariant = std::variant<
        BasicDescriptorSet,
        UnlitSingleTexture,
        ForwardShadingDstSet> ;

    struct DescriptorSetData
    {
        vk::DescriptorBufferInfo cameraUBOBuffer; // for camera uniform buffer

        vk::DescriptorBufferInfo pbrMaterialNoTexture; // for material uniform buffer
        vk::DescriptorBufferInfo pbrMaterialFeatures; // for material uniform buffer

        vk::DescriptorBufferInfo meshUBBOBuffer; // for mesh uniform buffer

        vk::DescriptorImageInfo diffuseTextureImage; // for diffuse
        vk::DescriptorImageInfo normalTextureImage; // for normal
        vk::DescriptorImageInfo armTextureImage; // for arm
        vk::DescriptorImageInfo emissiveTextureImage; // for emisive texture

        vk::DescriptorBufferInfo lightInformation; // all light information

        vk::DescriptorImageInfo LUT_LTC; // for linearly transformed cosines
        vk::DescriptorImageInfo LUT_LTC_Inverse; // for linearly transformed cosines
    };
}

#endif //VDESCRIPTORSETSTRUCTS_HPP
