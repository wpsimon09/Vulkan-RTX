//
// Created by wpsimon09 on 22/02/25.
//
#include "ModelManagmentUtils.hpp"

#include <cstdint>
#include <stb_image/stb_image_write.h>
#include <stb_image/stb_image.h>

#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"

namespace ApplicationCore
{
    VulkanStructs::ImageData<> LoadImage(const std::string &path, bool saveToDisk)
    {
        VulkanStructs::ImageData imageData{};

        imageData.pixels = reinterpret_cast<uint32_t *>(stbi_load(path.c_str(), &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha));
        imageData.channels = 4;
        auto imageName = path.substr(path.rfind("/") + 1);
        imageData.fileName = GlobalVariables::textureFolder / imageName;
        imageData.sourceType = EImageSource::File;
        auto folder = GlobalVariables::textureFolder.string();

        if (saveToDisk)
        {
            if (CheckIfImageExistsInFolader(GlobalVariables::textureFolder, imageData.fileName))
            {
                Utils::Logger::LogInfo("Image already exists in the folder, skipping saving");
            }
            else
            {
                SaveImageAsPNG(imageData.widht, imageData.height, imageData.channels, imageData.fileName, reinterpret_cast<std::vector<std::byte> &>(imageData.pixels));
            }
        }

        if (!imageData.pixels)
        {
            Utils::Logger::LogError("Failed to generate texture at path: \t" + path);
            Utils::Logger::LogInfo("Failing back to the default texture");

            imageData.pixels = reinterpret_cast<uint32_t *>(stbi_load("Resources/DefaultTexture.jpg", &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha));
            imageData.channels = 4;
            imageData.fileName = path;

            if (!imageData.pixels)
            {
                throw std::runtime_error("Fallback to default texture failed, this should never happen !");
            }
        }
        else
        {
            Utils::Logger::LogSuccess("Image at path:\t" + path + "\n read successfully");
        }
        //-> to test the concurrency uncomment this line
        // std::this_thread::sleep_for(std::chrono::seconds(7));

        return imageData;
    }

    VulkanStructs::ImageData<> LoadImage(const TextureBufferInfo &data, const std::string &textureID, bool saveToDisk)
    {
        VulkanStructs::ImageData imageData{};
        if (data.data)
        {

            if (
                (imageData.pixels = reinterpret_cast<uint32_t *>(stbi_load_from_memory(reinterpret_cast<stbi_uc *>(data.data), static_cast<int>(data.size), &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha))))
            {
                imageData.channels = 4;
                imageData.sourceType = EImageSource::Buffer;
                imageData.fileName = GlobalVariables::textureFolder.string() + textureID;

                if (saveToDisk)
                {
                    if (CheckIfImageExistsInFolader(GlobalVariables::textureFolder, imageData.fileName))
                    {
                        Utils::Logger::LogInfo("Image already exists in the folder, skipping saving");
                    }
                    else
                    {
                        SaveImageAsPNG(imageData.widht, imageData.height, imageData.channels, imageData.fileName, reinterpret_cast<std::vector<std::byte> &>(imageData.pixels));
                    }
                }
            }

            else
            {

                Utils::Logger::LogError("Failed to generate texture that was loaded from memory, textureID was:" + textureID);
                Utils::Logger::LogInfo("Failing back to the default texture");

                imageData.pixels = reinterpret_cast<uint32_t *>(stbi_load("Resources/DefaultTexture.jpg", &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha));
                imageData.channels = 4;
                imageData.fileName = "Resources/DefaultTexture.jpg";

                if (!imageData.pixels)
                {
                    throw std::runtime_error("Fallback to default texture failed, this should never happen !");
                }
            }
        }
        else{

            return {};
        }
        //-> to test the concurrency uncomment this line
        // std::this_thread::sleep_for(std::chrono::seconds(7));

        return imageData;
    }

    VulkanStructs::ImageData<float> LoadHDRImage(const std::string& path, bool saveToDisk)
    {
        VulkanStructs::ImageData<float> imageData{};

        imageData.pixels = reinterpret_cast<float *>(stbi_loadf(path.c_str(), &imageData.widht, &imageData.height, &imageData.channels, STBI_rgb_alpha));
        imageData.channels = 4;
        auto imageName = path.substr(path.rfind("/") + 1);
        imageData.fileName = GlobalVariables::textureFolder / imageName;
        imageData.sourceType = EImageSource::File;
        imageData.format  = vk::Format::eR32G32B32A32Sfloat;
        auto folder = GlobalVariables::textureFolder.string();

        if (saveToDisk)
        {
            if (CheckIfImageExistsInFolader(GlobalVariables::textureFolder, imageData.fileName))
            {
                Utils::Logger::LogInfo("Image already exists in the folder, skipping saving");
            }
            else
            {
                SaveImageAsPNG(imageData.widht, imageData.height, imageData.channels, imageData.fileName, reinterpret_cast<std::vector<std::byte> &>(imageData.pixels));
            }
        }

        if (!imageData.pixels)
        {
            throw std::runtime_error("Float texture not found, this has no fallback");
        }
        else
        {
            Utils::Logger::LogSuccess("Image at path:\t" + path + "\n read successfully");
        }
        //-> to test the concurrency uncomment this line
        // std::this_thread::sleep_for(std::chrono::seconds(7));

        return imageData;
    }

    void SaveImageAsPNG(int width, int height, int channels, const std::string &path,
                        const std::vector<std::byte> &data)
    {
        const unsigned char *imageData = reinterpret_cast<const unsigned char *>(data.data());
        int err = stbi_write_jpg(path.c_str(), width, height, channels, imageData, 100);
        if (err == 0)
        {
            Utils::Logger::LogSuccess("Image saved to " + path);
        }
        else
        {
            Utils::Logger::LogErrorClient("Failed to save image to : " + path + "with error code :" + std::to_string(err));
        }
    }

    bool CheckIfImageExistsInFolader(const std::filesystem::path &folder, const std::filesystem::path &image)
    {
        for (const auto &entry : std::filesystem::directory_iterator(folder))
        {
            if (entry.path().filename() == image.filename())
            {
                return true;
            }
        }
        return false;
    }

    glm::mat4 FastGLTFToGLMMat4(fastgltf::math::fmat4x4 *matrix)
    {
        glm::mat4 newMatrix;
        memcpy(&newMatrix, matrix->data(), sizeof(fastgltf::math::fmat4x4));
        return newMatrix;
    }
}
