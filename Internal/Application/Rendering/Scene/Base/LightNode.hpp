//
// Created by wpsimon09 on 06/02/25.
//

#ifndef LIGHTNODE_HPP
#define LIGHTNODE_HPP
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Scene/SceneNode.hpp"
#include "Vulkan/Global/RenderingOptions.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include "Vulkan/Utils/VRenderingContext/VRenderingContext.hpp"

namespace ApplicationCore {
template <typename T>
class LightNode : public SceneNode
{
public:
  LightNode(std::shared_ptr<StaticMesh> mesh, T* lightData = nullptr);

  void Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const override;
  void Update() override;
  void ProcessNodeRemove() override = 0;

  T&   GetLightStruct() { return m_lightStruct; };
  void SetVisualisationMesh(std::shared_ptr<StaticMesh> mesh);

protected:
  T                           m_lightStruct;
  std::shared_ptr<StaticMesh> m_visualisationMesh;
};

template <typename T>
LightNode<T>::LightNode(std::shared_ptr<StaticMesh> mesh, T* lightData)
    : SceneNode(mesh)
    , m_lightStruct()
{
  //checks if i provided any light data from the .ini file
  m_lightStruct = lightData != nullptr ? *lightData : m_lightStruct;
}

template <typename T>
void LightNode<T>::Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const
{
}


template <typename T>
void LightNode<T>::Update()
{
  SceneNode::Update();
}

template <typename T>
void LightNode<T>::SetVisualisationMesh(std::shared_ptr<StaticMesh> mesh)
{
  m_visualisationMesh = mesh;
}
}  // namespace ApplicationCore

#endif  //LIGHTNODE_HPP
