//
// Created by wpsimon09 on 22/02/25.
//

#ifndef MODELMANAGMENTUTILS_HPP
#define MODELMANAGMENTUTILS_HPP

#include "Application/VertexArray/VertexArray.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "filesystem"
#include "fastgltf/math.hpp"
#include <cstdint>
struct TextureBufferInfo;
namespace ApplicationCore {
VulkanStructs::VImageData<> LoadImage(const std::string& path, bool saveToDisk = false);
VulkanStructs::VImageData<> LoadImage(const TextureBufferInfo& data, const std::string& textureID, bool saveToDisk = false);
VulkanStructs::VImageData<float>              LoadHDRImage(const std::string& path, bool saveToDisk = false);
std::vector<VulkanStructs::VImageData<float>> LoadTextureArray(const std::string& folderName, bool saveToDisk = false);

void SaveImageAsPNG(int width, int height, int channels, const std::string& path, const std::vector<std::byte>& data);
void SaveImageAsHDR(int width, int height, int channels, const std::string& path, float* data);

bool      CheckIfImageExistsInFolader(const std::filesystem::path& folder, const std::filesystem::path& image);
glm::mat4 FastGLTFToGLMMat4(fastgltf::math::fmat4x4* matrix);

bool GeneratTangents(std::vector<ApplicationCore::Vertex>& vertices, std::vector<uint32_t>& indices);

}  // namespace ApplicationCore


#endif  //MODELMANAGMENTUTILS_HPP
