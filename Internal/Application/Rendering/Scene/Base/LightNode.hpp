//
// Created by wpsimon09 on 06/02/25.
//

#ifndef LIGHTNODE_HPP
#define LIGHTNODE_HPP
#include "Application/Rendering/Scene/SceneNode.hpp"

namespace ApplicationCore {

template<typename  T>
class LightNode: public SceneNode {
public:
    LightNode(std::shared_ptr<StaticMesh> mesh);

    void Render(VulkanStructs::RenderContext* renderingContext) const override = 0;
    void Update() const override;
    void ProcessNodeRemove() override = 0;

    T& GetLightStruct() {return m_lightStruct;};
protected:
    T m_lightStruct;
};

template <typename T>
LightNode<T>::LightNode(std::shared_ptr<StaticMesh> mesh):SceneNode(mesh), m_lightStruct()
{
}

template <typename T>
void LightNode<T>::Update() const
{
    SceneNode::Update();
}
} // ApplicationCore

#endif //LIGHTNODE_HPP
