//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_FILEHEADERS_HPP
#define VULKAN_RTX_FILEHEADERS_HPP
#include "Application/Utils/Uuid.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "vulkan/vulkan.hpp"

#include <glm/vec4.hpp>

namespace ApplicationCore {
struct IFileHeader
{
    uuid::UUID  uuid;
    std::string name;
};

struct VMaterialHeader : public IFileHeader
{
    float     roughness{0.4f};
    float     metalness{0.2f};
    float     ao{0.2f};
    float     padding{0.0f};
    glm::vec4 albedo{0.2f, 0.9f, 0.4f, 1.0f};
    glm::vec4 emissive_strength{0.0f, 0.0f, 0.0f, 0.0f};

    int hasAlbedoTexture{false};
    int hasEmissiveTexture{false};
    int hasNormalTexture{false};
    int hasArmTexture{false};

    //===========================================================
    // UUIDS of separate textures loaded to the assets browser
    uuid::UUID alebdoTexture;
    uuid::UUID armTextureIdx;
    uuid::UUID emissiveTextureIdx;
    uuid::UUID normalTextureIdx;
};

struct VMeshHeader : public IFileHeader
{
    uint32_t vertexCount{0};
    uint32_t indexCount{0};
};

struct VTextureHeader : public IFileHeader
{
    int          widht, height, channels;
    std::string  fileName;
    EImageSource sourceType = EImageSource::Generated;
    vk::Format   format     = vk::Format::eR8G8B8A8Unorm;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_FILEHEADERS_HPP
