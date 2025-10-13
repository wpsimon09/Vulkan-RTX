//
// Created by wpsimon09 on 28/09/2025.
//

#ifndef VULKAN_RTX_ATMOSPHERESCENENODE_HPP
#define VULKAN_RTX_ATMOSPHERESCENENODE_HPP
#include "Application/Structs/ParameterStructs.hpp"
#include "SceneNode.hpp"

namespace ApplicationCore {

class AtmosphereSceneNode : public SceneNode
{
  public:
    explicit AtmosphereSceneNode(std::shared_ptr<ApplicationCore::StaticMesh> mesh);

    void Update(SceneUpdateContext& sceneUpdateFlags) override;
    void Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const override;

    AtmosphereParameters&       GetParameters();
    void                        ProcessNodeRemove() override;
    void                        ProcessNodeRemove(SceneData& sceneData) override;
    void                        ProcessNodeRemove(const SceneNode& node, SceneData& sceneData) override;
    const AtmosphereParameters& GetDefaultAtmosphereParams();

  private:
    const VulkanStructs::VDrawCallData m_drawCall;

    AtmosphereParameters       m_parameters;
    const AtmosphereParameters m_defaultParmas;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_ATMOSPHERESCENENODE_HPP
