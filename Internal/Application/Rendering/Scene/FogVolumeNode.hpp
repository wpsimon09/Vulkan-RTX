//
// Created by wpsimon09 on 23/06/25.
//

#ifndef FOGVOLUMENODE_HPP
#define FOGVOLUMENODE_HPP
#include "SceneNode.hpp"

namespace ApplicationCore {

class FogVolumeNode: public SceneNode {

public:
  explicit FogVolumeNode();

  void Update(SceneUpdateFlags& sceneUpdateFlags) override;
  void Render(ApplicationCore::EffectsLibrary& effectsLibrary, VulkanUtils::RenderContext* renderingContext) const override;

    m_fogVolumeParameters& GetParameters();

  private:
    m_fogVolumeParameters m_parameters;
};

} // ApplicationCore

#endif //FOGVOLUME_HPP
