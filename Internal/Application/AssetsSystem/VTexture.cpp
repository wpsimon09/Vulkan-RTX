//
// Created by simon on 18/01/2026.
//

#include "VTexture.hpp"

namespace ApplicationCore {
VTexture::VTexture()
    : VAsset2<ApplicationCore::VTextureHeader>(".Vtex")
{
}
VTexture::VTexture(std::filesystem::path& path, VTextureHeader& header, void* pixels)
    : VAsset2<ApplicationCore::VTextureHeader>(".Vtex")
{
}
bool     VTexture::Save(std::filesystem::path& path) {}
bool     VTexture::Load() {}
std::any VTexture::LoadData() {}
}  // namespace ApplicationCore