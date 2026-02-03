//
// Created by simon on 18/01/2026.
//

#include "VMaterial.hpp"

#include <fstream>

namespace ApplicationCore {
VMaterial::VMaterial()
    : VAsset2<ApplicationCore::VMaterialHeader, int>(".VMat")
{
}

VMaterial::VMaterial(AssetEntry& databaseEntry)
    : VAsset2<ApplicationCore::VMaterialHeader, int>(databaseEntry, ".VMat")
{
    LoadHeader();
}
VMaterial::VMaterial(AssetEntry& databaseEntry, VMaterialHeader& materialData)
    : VAsset2<ApplicationCore::VMaterialHeader, int>(databaseEntry, ".VMat")
{
    m_fileHeader      = materialData;
    m_fileHeader.uuid = databaseEntry.uuid;

    std::ofstream file(m_path, std::ios::binary);

    if(file.is_open())
    {
        // the material file just store its data
        m_fileHeader.Serialize(file);
        file.close();
    }
}
bool     VMaterial::Save(std::filesystem::path& path) {}
bool     VMaterial::Load() {}
std::any VMaterial::LoadData() {}
}  // namespace ApplicationCore