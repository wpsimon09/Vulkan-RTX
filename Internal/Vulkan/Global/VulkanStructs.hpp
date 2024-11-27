//
// Created by wpsimon09 on 20/11/24.
//

#ifndef VULKANSTRUCTS_HPP
#define VULKANSTRUCTS_HPP

#include <stb_image/stb_image.h>
#include <vulkan/vulkan.hpp>

namespace VulkanStructs
{
    struct ImageData
    {
        uint32_t* pixels;
        int widht, height, channels;

        size_t GetSize() const {
            return widht * height * channels;
        }
        void Clear() const {
            stbi_image_free(pixels);
        }
    };

    struct DrawCallData
    {
        uint32_t indexCount = 0;
        uint32_t firstIndex =0;
        VkBuffer& vertexBuffer;
        VkBuffer& indexBuffer;
        glm::mat4& modelMatrix;
        uint32_t instanceCount = 1;
        // material descriptor

    };

    struct RenderContext
    {
        // Pipeline
        std::vector<DrawCallData> DrawCalls;
    };
}

#endif //VULKANSTRUCTS_HPP
