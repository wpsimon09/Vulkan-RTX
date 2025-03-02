//
// Created by wpsimon09 on 04/02/25.
//

#ifndef POINTLIGHTNODE_HPP
#define POINTLIGHTNODE_HPP
#include "SceneNode.hpp"
#include "Application/Structs/LightStructs.hpp"
#include "Base/LightNode.hpp"

namespace ApplicationCore {

class PointLightNode:public LightNode<LightStructs::PointLight> {
public:
    explicit PointLightNode(LightStructs::SceneLightInfo& sceneLightInfo,std::shared_ptr<StaticMesh> mesh);
    explicit PointLightNode(LightStructs::SceneLightInfo& sceneLightInfo,std::shared_ptr<StaticMesh> mesh, LightStructs::PointLight * pointLightData = nullptr);

    void Render(VulkanStructs::RenderContext* renderingContext) const override;
    void Update() override;
    void ProcessNodeRemove() override;
private:
    LightStructs::SceneLightInfo& m_sceneLightInfo;
    int m_index;
};

} // ApplicationCore

#endif //POINTLIGHTNODE_HPP
