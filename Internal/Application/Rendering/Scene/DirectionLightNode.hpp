//
// Created by wpsimon09 on 26/01/25.
//

#ifndef DRECTIONAL_LIGHT_NODE_H
#define DRECTIONAL_LIGHT_NODE_H
#include "SceneNode.hpp"
#include "Application/Structs/LightStructs.hpp"
#include "Base/LightNode.hpp"
#include "Base/LightNode.hpp"

namespace ApplicationCore {

class DirectionLightNode: public ApplicationCore::LightNode<LightStructs::DirectionalLight> {
public:
    explicit DirectionLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh);

    void Render(VulkanStructs::RenderContext* renderingContext) const override;
    void Update() const override;
    void ProcessNodeRemove() override;
};

} // ApplicationCore

#endif //LIGHTNODE_HPP
