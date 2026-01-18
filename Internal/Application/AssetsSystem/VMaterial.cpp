//
// Created by simon on 18/01/2026.
//

#include "VMaterial.hpp"

namespace ApplicationCore {
VMaterial::VMaterial()
    : VAsset2<ApplicationCore::VMaterialHeader>(".Vmat")
{
}
VMaterial::VMaterial(std::filesystem::path& path, VMaterialHeader& materialData)
    : VAsset2<ApplicationCore::VMaterialHeader>(".Vmat")
{
    m_fileHeader = materialData;
}
bool     VMaterial::Save(std::filesystem::path& path) {}
bool     VMaterial::Load() {}
std::any VMaterial::LoadData() {}
}  // namespace ApplicationCore