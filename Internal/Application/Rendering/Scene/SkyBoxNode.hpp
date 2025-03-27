//
// Created by wpsimon09 on 27/03/25.
//

#ifndef SKYBOXNODE_HPP
#define SKYBOXNODE_HPP
#include "SceneNode.hpp"

namespace ApplicationCore {

class SkyBoxNode:public SceneNode {
    public:
        explicit SkyBoxNode(std::shared_ptr<StaticMesh> mesh);
        void Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const override;
};

} // ApplicationCore

#endif //SKYBOXNODE_HPP
