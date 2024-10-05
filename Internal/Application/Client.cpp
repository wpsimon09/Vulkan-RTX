//
// Created by wpsimon09 on 04/10/24.
//

#include "Client.hpp"

#include <cassert>

#include "Logger/Logger.hpp"
#include "Rendering/Mesh/Mesh.hpp"

void Client::Init() {

    auto start = std::chrono::high_resolution_clock::now();


    m_meshes.push_back(std::make_unique<ApplicationCore::Mesh>(MESH_GEOMETRY_PLANE));
    assert(!m_meshes.empty());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    Utils::Logger::LogSuccess("Client side initialized in: " +  std::to_string(duration.count()) + "seconds");
}

void Client::Update() {
    // TODO: camera updates, scene movement, proecessing user input etc...
}
