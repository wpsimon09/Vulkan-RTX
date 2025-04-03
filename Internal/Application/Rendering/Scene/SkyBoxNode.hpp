//
// Created by wpsimon09 on 27/03/25.
//

#ifndef SKYBOXNODE_HPP
#define SKYBOXNODE_HPP
#include "SceneNode.hpp"
#include "Application/Lightning/LightStructs.hpp"
#include "Base/LightNode.hpp"

namespace ApplicationCore {

class SkyBoxNode:public LightNode<LightStructs::EnvLight> {
    public:
        explicit SkyBoxNode(LightStructs::SceneLightInfo& sceneLightInfo, std::shared_ptr<StaticMesh> mesh);
        void Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const override;
        void ProcessNodeRemove() override;
        void Update() override;
};

} // ApplicationCore

#endif //SKYBOXNODE_HPP
