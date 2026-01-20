//
// Created by simon on 18/01/2026.
//

#include "VMesh.hpp"

#include <fstream>

namespace ApplicationCore {
VMesh::VMesh(AssetEntry& databaseEntry)
    : VAsset2<ApplicationCore::VMeshHeader>(databaseEntry, ".VMesh")
{
}

VMesh::VMesh(AssetEntry& databaseEntry, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
    : VAsset2<ApplicationCore::VMeshHeader>(databaseEntry, ".VMesh")
{
    //==================================
    // save to file and delete the object
    m_fileHeader.uuid        = databaseEntry.uuid;
    m_fileHeader.name        = databaseEntry.name;
    m_fileHeader.indexCount  = indices.size();
    m_fileHeader.vertexCount = vertices.size();

    std::ofstream file(m_path, std::ios::binary);

    if(file.is_open())
    {
        file.write(reinterpret_cast<char*>(&m_fileHeader), sizeof(m_fileHeader));

        // Write vertex data
        file.write(reinterpret_cast<const char*>(vertices.data()), sizeof(Vertex) * vertices.size());
        // Write index data
        file.write(reinterpret_cast<const char*>(indices.data()), sizeof(uint32_t) * indices.size());

        file.close();
    }
}
bool     VMesh::Save(std::filesystem::path& path) {}
bool     VMesh::Load() {}
std::any VMesh::LoadData() {}
}  // namespace ApplicationCore