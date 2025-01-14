//
// Created by wpsimon09 on 04/10/24.
//

#ifndef VERTEXARRAY_HPP
#define VERTEXARRAY_HPP

#include <memory>
#include <optional>
#include <vector>
#include "Application/Enums/ClientEnums.hpp"
#include "glm/glm.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"



namespace VulkanCore
{
    class VDevice;
    class VBuffer;
}

namespace ApplicationCore
{
    struct Vertex
    {
        glm::vec3 position = {0.0f, 0.0F, 0.0f};
        glm::vec3 normal {0.0f, 1.0f, 0.0f};
        glm::vec2 uv = {0.0f, 0.0f};
    };

    class VertexArray {
    public:
        explicit VertexArray(VulkanStructs::MeshData& meshData);

        VulkanStructs::MeshData* GetMeshData() { return &m_meshData; }
        uint32_t GetAABBIndexCount() const { return 36; }

        void Destroy() const;;

        int GetAttributeCount() {return 3;};

         ~VertexArray() = default;

    private:
        // TODO : instead of having this VBuffer unique it can be made a const reference tho the vkBuffer allocated at once

        VulkanStructs::MeshData m_meshData;
        PRIMITIVE_TOPOLOGY m_topology;
    };
}



#endif //VERTEXARRAY_HPP
