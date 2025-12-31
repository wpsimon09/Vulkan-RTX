//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_WORLD_HPP
#define VULKAN_RTX_WORLD_HPP
#include <memory>

namespace ECS {
class Coordinator;
}
namespace ApplicationCore {
class ApplicationState;

class World
{
  public:
    explicit World(ApplicationState& applicationState);

    void RegisterAll();

    ECS::Coordinator& GetECS();

  private:
    ApplicationState&                 m_applicationState;
    std::unique_ptr<ECS::Coordinator> m_ecsCoordinator;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_WORLD_HPP
