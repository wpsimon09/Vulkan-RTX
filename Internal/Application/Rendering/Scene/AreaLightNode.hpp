//
// Created by wpsimon09 on 09/02/25.
//

#ifndef AREALIGHTNODE_HPP
#define AREALIGHTNODE_HPP

#include "Base/LightNode.hpp"

namespace VulkanStructs{
    struct RenderContext;
}

namespace ApplicationCore {

class AreaLightNode : public LightNode<LightStructs::AreaLight> {
public:
    explicit AreaLightNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh);

    void Render(VulkanStructs::RenderContext* renderingContext) const override;
    void Update()  override;
    void ProcessNodeRemove() override;
private:
    int m_index;
    LightStructs::SceneLightInfo& m_sceneLightInfo;
};

} // ApplicationCore

#endif //AREALIGHTNODE_HPP
