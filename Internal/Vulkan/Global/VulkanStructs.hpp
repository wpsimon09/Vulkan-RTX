//
// Created by wpsimon09 on 20/11/24.
//

#ifndef VULKANSTRUCTS_HPP
#define VULKANSTRUCTS_HPP

#include <stb_image/stb_image.h>
#include <vulkan/vulkan.hpp>

namespace VulkanStrucuts
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

    struct DrawCall
    {
        uint32_t indexCount;
        uint32_t firstIndex;


    };

    struct RenderContext
    {
        std::vector<DrawCall> DrawCalls;
    };
}

#endif //VULKANSTRUCTS_HPP
