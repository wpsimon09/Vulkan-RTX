//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_FILEHEADERS_HPP
#define VULKAN_RTX_FILEHEADERS_HPP
#include "Application/Utils/ApplicationUtils.hpp"
#include "Application/Utils/Uuid.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "vulkan/vulkan.hpp"

#include <glm/vec4.hpp>

namespace ApplicationCore {
struct IFileHeader
{
    uuid::UUID  uuid;
    std::string name;

    void Serialize(std::ostream& out) const
    {
        WriteString(out, uuid);
        WriteString(out, name);
    }

    void Deserialize(std::istream& in)
    {
        ReadString(in, uuid);
        ReadString(in, name);
    }
};

struct VMaterialHeader : public IFileHeader
{
    float     roughness{0.4f};
    float     metalness{0.2f};
    float     ao{0.2f};
    float     padding{0.0f};
    glm::vec4 albedo{0.2f, 0.9f, 0.4f, 1.0f};
    glm::vec4 emissive_strength{0.0f};

    int hasAlbedoTexture{false};
    int hasEmissiveTexture{false};
    int hasNormalTexture{false};
    int hasArmTexture{false};

    uuid::UUID alebdoTexture;
    uuid::UUID armTextureIdx;
    uuid::UUID emissiveTextureIdx;
    uuid::UUID normalTextureIdx;

    void Serialize(std::ostream& out) const
    {
        IFileHeader::Serialize(out);

        WritePod(out, roughness);
        WritePod(out, metalness);
        WritePod(out, ao);
        WritePod(out, padding);
        WritePod(out, albedo);
        WritePod(out, emissive_strength);

        WritePod(out, hasAlbedoTexture);
        WritePod(out, hasEmissiveTexture);
        WritePod(out, hasNormalTexture);
        WritePod(out, hasArmTexture);

        WritePod(out, alebdoTexture);
        WritePod(out, armTextureIdx);
        WritePod(out, emissiveTextureIdx);
        WritePod(out, normalTextureIdx);
    }

    void Deserialize(std::istream& in)
    {
        IFileHeader::Deserialize(in);

        ReadPod(in, roughness);
        ReadPod(in, metalness);
        ReadPod(in, ao);
        ReadPod(in, padding);
        ReadPod(in, albedo);
        ReadPod(in, emissive_strength);

        ReadPod(in, hasAlbedoTexture);
        ReadPod(in, hasEmissiveTexture);
        ReadPod(in, hasNormalTexture);
        ReadPod(in, hasArmTexture);

        ReadPod(in, alebdoTexture);
        ReadPod(in, armTextureIdx);
        ReadPod(in, emissiveTextureIdx);
        ReadPod(in, normalTextureIdx);
    }
};

struct VMeshHeader : public IFileHeader
{
    uint32_t   vertexCount{0};
    uint32_t   indexCount{0};
    uuid::UUID materialId{0};

    void Serialize(std::ostream& out) const
    {
        IFileHeader::Serialize(out);
        WritePod(out, vertexCount);
        WritePod(out, indexCount);
        WritePod(out, materialId);
    }

    void Deserialize(std::istream& in)
    {
        IFileHeader::Deserialize(in);
        ReadPod(in, vertexCount);
        ReadPod(in, indexCount);
        ReadPod(in, materialId);
    }
};

struct VTextureHeader : public IFileHeader
{
    int          widht{0}, height{0}, channels{0};
    std::string  fileName;
    EImageSource sourceType = EImageSource::Generated;
    vk::Format   format     = vk::Format::eR8G8B8A8Unorm;

    void Serialize(std::ostream& out) const
    {
        IFileHeader::Serialize(out);

        WritePod(out, widht);
        WritePod(out, height);
        WritePod(out, channels);

        WriteString(out, fileName);
        WritePod(out, sourceType);
        WritePod(out, format);
    }

    void Deserialize(std::istream& in)
    {
        IFileHeader::Deserialize(in);

        ReadPod(in, widht);
        ReadPod(in, height);
        ReadPod(in, channels);

        ReadString(in, fileName);
        ReadPod(in, sourceType);
        ReadPod(in, format);
    }
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_FILEHEADERS_HPP
