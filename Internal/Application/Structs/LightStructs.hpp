//
// Created by wpsimon09 on 03/02/25.
//

#ifndef LIGHTSTRUCTS_HPP
#define LIGHTSTRUCTS_HPP

#include <array>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace LightStructs {
struct AreaLight {
  glm::vec3 colour;
  float intensity;

  bool twoSided;
  std::array<glm::vec4, 4> edges = {
      glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f}, // First point (bottom-left corner)
      glm::vec4{0.5f, -0.5f, 0.0f, 1.0f},  // Second point (bottom-right corner)
      glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f},  // Third point (top-left corner)
      glm::vec4{0.5f, 0.5f, 0.0f, 1.0f}};

  bool isAreaLightInUse = false;

  void Reset() {
    colour = glm::vec3(0.f);
    intensity = 0.f;
    twoSided = false;
    isAreaLightInUse = false;
    edges = {
        glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f}, // First point (bottom-left corner)
        glm::vec4{0.5f, -0.5f, 0.0f,
                  1.0f}, // Second point (bottom-right corner)
        glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f}, // Third point (top-left corner)
        glm::vec4{0.5f, 0.5f, 0.0f, 1.0f}};
  }

  std::array<glm::vec4, 4> GetAreaLightEdges() {
    return {
        glm::vec4{1.0f, -0.5f, 1.0f, 1.0f}, // First point (bottom-left corner)
        glm::vec4{-1.0f, -0.5f, 1.0f, 1.0f}, // Second point (bottom-right corner)
        glm::vec4{-1.0f, -0.5f, -1.0f, 1.0f}, // Third point (top-left corner)
        glm::vec4{1.0f, -0.5f, -1.0f, 1.0f}};
  }
};

struct PointLight {
  mutable glm::vec4 colour;

  mutable glm::vec3 position;

  float constantFactor = 1.0f;

  float linearFactor = 0.045f;

  float quadraticFactor = 0.0075f;

  bool useAdvancedAttentuation = 0;
  bool isPointLightInUse = false;

  void Reset() {
    colour = glm::vec4(0.f);
    position = glm::vec3(0.f);
    constantFactor = 1.0f;
    linearFactor = 0.045f;
    quadraticFactor = 0.0075f;
    isPointLightInUse = false;
  }
};

struct DirectionalLight {
  glm::vec4 colour;

  mutable glm::vec3 direction;

  void Reset() {
    colour = glm::vec4(0.f);
    direction = glm::vec3(0.f);
  }
};

struct SceneLightInfo {
  LightStructs::DirectionalLight *DirectionalLightInfo = nullptr;

  std::vector<LightStructs::PointLight *> PointLightInfos;
  std::vector<LightStructs::AreaLight *> AreaLightInfos;

  int AddPointLight(PointLight *pointLight) {
    PointLightInfos.emplace_back(pointLight);
    pointLight->isPointLightInUse = true;
    CurrentPointLightIndex++;
    return CurrentPointLightIndex - 1;
  };

  int AddAreaLight(AreaLight *areaLight) {
    AreaLightInfos.emplace_back(areaLight);
    areaLight->isAreaLightInUse = true;
    CurrentAreaLightIndex++;
    return CurrentPointLightIndex - 1;
  };

private:
  int CurrentPointLightIndex = 0;
  int CurrentAreaLightIndex = 0;
};

} // namespace LightStructs

#endif // LIGHTSTRUCTS_HPP
