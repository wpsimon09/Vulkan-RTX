//
// Created by wpsimon09 on 04/02/25.
//

#ifndef POINTLIGHTNODE_HPP
#define POINTLIGHTNODE_HPP
#include "SceneNode.hpp"
#include "Application/Structs/LightStructs.hpp"

namespace ApplicationCore {

class PointLightNode:public SceneNode {
public:
    explicit PointLightNode(LightStructs::SceneLightInfo& sceneLightInfo,std::shared_ptr<StaticMesh> mesh);

    void Render(VulkanStructs::RenderContext* renderingContext) const override;
    void Update() const override;
    void ProcessNodeRemove() override;
    LightStructs::PointLight& GetLightStruct(){return m_lightStruct;};
private:
    LightStructs::PointLight m_lightStruct;
    LightStructs::SceneLightInfo& m_sceneLightInfo;
    int m_index;
};

} // ApplicationCore

#endif //POINTLIGHTNODE_HPP
