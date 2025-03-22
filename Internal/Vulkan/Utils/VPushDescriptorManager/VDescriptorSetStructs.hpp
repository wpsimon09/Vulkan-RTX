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
        vk::DescriptorBufferInfo buffer1; // for camera uniform buffer
        vk::DescriptorBufferInfo buffer2; // for mesh uniform buffer
        vk::DescriptorBufferInfo buffer3;

        virtual ~BasicDescriptorSet() = default;
    };

    struct UnlitSingleTexture
    {
        vk::DescriptorBufferInfo buffer1; // for camera uniform buffer
        vk::DescriptorBufferInfo buffer2; // for mesh uniform buffer
        vk::DescriptorBufferInfo buffer3;

        vk::DescriptorImageInfo texture2D_1;
        // ohter look up tables, irradiance, radiance maps etc...
    };

    struct ForwardShadingDstSet
    {
        vk::DescriptorBufferInfo buffer1; // for camera uniform buffer
        vk::DescriptorBufferInfo buffer2; // for mesh uniform buffer
        vk::DescriptorBufferInfo buffer3; // extra buffer

        vk::DescriptorBufferInfo buffer4; // for material uniform buffer
        vk::DescriptorBufferInfo buffer5; // for material uniform buffer
        vk::DescriptorBufferInfo buffer6; // all light information

        vk::DescriptorImageInfo texture2D_1; // for diffuse
        vk::DescriptorImageInfo texture2D_2; // for normal
        vk::DescriptorImageInfo texture2D_3; // for arm
        vk::DescriptorImageInfo texture2D_4; // for emisive texture

        vk::DescriptorImageInfo texture2D_5; // for linearly transformed cosines
        vk::DescriptorImageInfo texture2D_6; // for linearly transformed cosines

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
