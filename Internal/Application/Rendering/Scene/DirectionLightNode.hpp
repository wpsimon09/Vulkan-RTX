//
// Created by wpsimon09 on 26/01/25.
//

#ifndef LIGHTNODE_HPP
#define LIGHTNODE_HPP
#include "SceneNode.hpp"
#include "Application/Structs/LightStructs.hpp"

namespace ApplicationCore {

class DirectionLightNode: public SceneNode {
public:
    DirectionLightNode(std::shared_ptr<StaticMesh> mesh);

    void Render(VulkanStructs::RenderContext* renderingContext) const override;

    void Update() const override;
private:
    LightStructs::DirectionalLight m_lightStruct;
    glm::vec3 m_colour;
    float m_intensity;
};

} // ApplicationCore

#endif //LIGHTNODE_HPP
