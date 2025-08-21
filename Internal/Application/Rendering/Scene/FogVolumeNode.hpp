//
// Created by wpsimon09 on 23/06/25.
//

#ifndef FOGVOLUMENODE_HPP
#define FOGVOLUMENODE_HPP
#include "SceneNode.hpp"

namespace ApplicationCore {

class FogVolumeNode : public SceneNode
{

  public:
    explicit FogVolumeNode(std::shared_ptr<ApplicationCore::StaticMesh> mesh);

    void Update(SceneUpdateContext& sceneUpdateFlags) override;
    void Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const override;

    FogVolumeParameters& GetParameters();
    void                 ProcessNodeRemove() override;
    void                 ProcessNodeRemove(SceneData& sceneData) override;
    void                 ProcessNodeRemove(const SceneNode& node, SceneData& sceneData) override;

  private:
    FogVolumeParameters m_parameters;
};

}  // namespace ApplicationCore

#endif  //FOGVOLUME_HPP
