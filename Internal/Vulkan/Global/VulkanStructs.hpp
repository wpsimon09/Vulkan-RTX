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
        uint32_t indexCount;
        uint32_t firstIndex;
        std::vector<ApplicationCore::Vertex> vertices;
        std::vector<uint32_t> indices;
        // material
    };

    struct RenderContext
    {
        // Pipeline
        std::vector<DrawCallData> DrawCalls;
    };
}

#endif //VULKANSTRUCTS_HPP
