//
// Created by wpsimon09 on 09/02/25.
//

#ifndef AREALIGHTNODE_HPP
#define AREALIGHTNODE_HPP

#include "Base/LightNode.hpp"
#include "Application/Lightning/LightStructs.hpp"

namespace ApplicationCore {

class AreaLightNode : public LightNode<LightStructs::AreaLight>
{
  public:
    explicit AreaLightNode(LightStructs::SceneLightInfo& sceneLightInfo,
                           std::shared_ptr<StaticMesh>   mesh,
                           LightStructs::AreaLight*      areaLightData = nullptr);

    void Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const override;
    void Update() override;
    void ProcessNodeRemove() override;

  private:
    int m_index;

    LightStructs::SceneLightInfo& m_sceneLightInfo;
};

}  // namespace ApplicationCore

#endif  //AREALIGHTNODE_HPP
