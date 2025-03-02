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
    LightNode(std::shared_ptr<StaticMesh> mesh, T* lightData = nullptr);

    void Render(VulkanStructs::RenderContext* renderingContext) const override = 0;
    void Update()  override;
    void ProcessNodeRemove() override = 0;

    T& GetLightStruct() {return m_lightStruct;};
protected:
    T m_lightStruct;
};

template <typename T>
LightNode<T>::LightNode(std::shared_ptr<StaticMesh> mesh,T* lightData):SceneNode(mesh), m_lightStruct()
{
    m_lightStruct = lightData !=nullptr ? *lightData : m_lightStruct;
}

template <typename T>
void LightNode<T>::Update()
{
    SceneNode::Update();
}
} // ApplicationCore

#endif //LIGHTNODE_HPP
