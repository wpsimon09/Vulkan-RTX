//
// Created by wpsimon09 on 20/11/24.
//

#ifndef VULKANSTRUCTS_HPP
#define VULKANSTRUCTS_HPP

#include <stb_image/stb_image.h>
#include <vulkan/vulkan.hpp>

#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include "VMA/vk_mem_alloc.h"
#include "glm/glm.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp"


namespace ApplicationCore {
class BaseMaterial;
}

namespace VulkanUtils {
class VEffect;
}

namespace ApplicationCore {
class EffectsLibrary;
}

namespace LightStructs {
struct SceneLightInfo;
}

namespace VulkanStructs {
template <typename T = uint32_t>
struct ImageData
{
    T*           pixels;
    int          widht, height, channels;
    std::string  fileName   = "generated by Vulkan";
    EImageSource sourceType = EImageSource::Generated;
    vk::Format   format     = vk::Format::eR8G8B8A8Unorm;

    std::optional<size_t> offset = 0;


    size_t GetSize() const
    {
        if(std::is_same<T, uint32_t>::value)
            return widht * height * sizeof(T);
        else if(std::is_same<T, float>::value)
            return widht * height * channels * sizeof(float);
        else
            throw std::invalid_argument("Wrong image format supplied, supported formats are float and uint32_t");
    }
    void Clear() const
    {
        if(pixels && EImageSource::File)
        {
            stbi_image_free(pixels);
        }
    }
};


struct Bounds
{
    glm::vec3                origin;
    glm::vec3                extents;
    glm::vec3                max     = {-1.5f, -1.5f, -1.5f};  // min point in world space
    glm::vec3                min     = {1.5f, 1.5f, 1.5f};     // max point in world space
    std::array<glm::vec3, 8> corners = {
        glm::vec3{1, 1, 1},  glm::vec3{1, 1, -1},  glm::vec3{1, -1, 1},  glm::vec3{1, -1, -1},
        glm::vec3{-1, 1, 1}, glm::vec3{-1, 1, -1}, glm::vec3{-1, -1, 1}, glm::vec3{-1, -1, -1},

    };

    void ProjectToWorld(const glm::mat4& modelMatrix)
    {
        for(auto corner : corners)
        {
            corner = modelMatrix * glm::vec4(origin + (corner * extents), 1.0f);

            min = glm::min(min, corner);
            max = glm::max(max, corner);
        }
    }

    float radius;
};


struct RenderingStatistics
{
    int DrawCallCount = 0;
};

// holds offset to the larger buffer that is in GPU to prevent fragmentation
struct GPUSubBufferInfo
{
    vk::DeviceSize size;
    vk::DeviceSize offset;

    vk::Buffer        buffer;
    int               ID;
    int               BufferID;
    vk::DeviceAddress bufferAddress;

    bool operator==(const GPUSubBufferInfo& other) const { return BufferID == other.BufferID; }

    bool operator!=(const GPUSubBufferInfo& other) const { return BufferID != other.BufferID; }
};
;

struct MeshData
{
    GPUSubBufferInfo vertexData;
    GPUSubBufferInfo indexData;
    GPUSubBufferInfo vertexData_BB;
    GPUSubBufferInfo indexData_BB;
    Bounds           bounds;
};

struct GPUBufferInfo
{
    vk::DeviceSize size          = 0;
    vk::DeviceSize currentOffset = 0;
    vk::DeviceSize copyOffSet    = 0;
    vk::DeviceSize baseOffset    = 0;

    vk::Buffer           bufferVK;
    vk::BufferUsageFlags usageFlags;
    vk::DeviceAddress    bufferAddress;
    VkBuffer             bufferVMA;
    VmaAllocation        allocationVMA;

    int ID;

    vk::DeviceSize GetAvailableSize() const { return (currentOffset >= size) ? 0 : (size - currentOffset); }
    bool           WillNewBufferFit(vk::DeviceSize size) const { return size <= GetAvailableSize(); }
};

template <typename T>
struct ReadBackBufferInfo
{
    std::vector<T> data;
    int            bufferID;
    size_t         size;
};

struct StagingBufferInfo
{
    void*          mappedPointer;
    vk::DeviceSize size;
    vk::DeviceSize dstOffset;
    VmaAllocation  m_stagingAllocation;
    VkBuffer       m_stagingBufferVMA;
    vk::Buffer     m_stagingBufferVK;

    VkBuffer copyDstBuffer;
};

struct DrawCallData
{
    uint32_t indexCount    = 0;
    uint32_t firstIndex    = 0;
    uint32_t indexCount_BB = 36;
    uint32_t instanceCount = 1;

    GPUSubBufferInfo* vertexData = nullptr;
    GPUSubBufferInfo* indexData  = nullptr;
    Bounds*           bounds     = nullptr;


    mutable int drawCallID = 0;

    glm::mat4 modelMatrix;
    glm::vec3 position;

    float depth = -1.0f;

    unsigned long key = 0;

    bool inDepthPrePass = true;
    bool selected       = false;

    ApplicationCore::BaseMaterial*        material;
    std::shared_ptr<VulkanUtils::VEffect> effect;

    friend bool operator==(const DrawCallData& lhs, const ObjectDataUniform& rhs)
    {
        if(auto* lhsPBRMat = dynamic_cast<ApplicationCore::PBRMaterial*>(lhs.material))
        {
            auto& lhsMatDescription = lhsPBRMat->GetMaterialDescription();
            auto& rhsMatDescription = rhs.material;

            return lhsMatDescription.features == rhsMatDescription.features
                   && lhsMatDescription.values == rhsMatDescription.values && lhs.modelMatrix == rhs.model
                   && lhs.position == rhs.position;
        }
        else
        {
            return lhs.modelMatrix == rhs.model && lhs.position == rhs.position;
        }
    }

    friend bool operator!=(const DrawCallData& lhs, const ObjectDataUniform& rhs) { return !(lhs == rhs); }
};


}  // namespace VulkanStructs

#endif  //VULKANSTRUCTS_HPP
