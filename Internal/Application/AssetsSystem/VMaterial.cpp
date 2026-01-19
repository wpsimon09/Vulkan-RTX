//
// Created by simon on 18/01/2026.
//

#include "VMaterial.hpp"

namespace ApplicationCore {
VMaterial::VMaterial(AssetEntry& databaseEntry)
    : VAsset2<ApplicationCore::VMaterialHeader>(databaseEntry, ".VMat")
{
}
VMaterial::VMaterial(AssetEntry& databaseEntry, VMaterialHeader& materialData)
    : VAsset2<ApplicationCore::VMaterialHeader>(databaseEntry, ".VMat")
{
    m_fileHeader = materialData;
}
bool     VMaterial::Save(std::filesystem::path& path) {}
bool     VMaterial::Load() {}
std::any VMaterial::LoadData() {}
}  // namespace ApplicationCore