//
// Created by wpsimon09 on 22/02/25.
//

#ifndef MODELMANAGMENTUTILS_HPP
#define MODELMANAGMENTUTILS_HPP

#include "Vulkan/Global/VulkanStructs.hpp"
#include "filesystem"
#include "fastgltf/math.hpp"
struct TextureBufferInfo;
namespace ApplicationCore
{
    VulkanStructs::ImageData<> LoadImage(const std::string &path, bool saveToDisk = false);
    VulkanStructs::ImageData<> LoadImage(const TextureBufferInfo& data, const std::string& textureID, bool saveToDisk = false);
    VulkanStructs::ImageData<float> LoadHDRImage(const std::string &path, bool saveToDisk = false);

    void SaveImageAsPNG(int width, int height, int channels, const std::string& path, const std::vector<std::byte>& data);
    void SaveImageAsHDR(int width, int height, int channels, const std::string& path, const std::vector<float>& data);

    bool CheckIfImageExistsInFolader(const std::filesystem::path& folder,const std::filesystem::path& image);
    glm::mat4 FastGLTFToGLMMat4(fastgltf::math::fmat4x4* matrix);

}


#endif //MODELMANAGMENTUTILS_HPP
