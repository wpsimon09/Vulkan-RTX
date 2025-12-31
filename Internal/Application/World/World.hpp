//
// Created by simon on 31/12/2025.
//

#ifndef VULKAN_RTX_WORLD_HPP
#define VULKAN_RTX_WORLD_HPP
#include <memory>

namespace ECS {
class ECSCoordinator;
}
namespace ApplicationCore {
class ApplicationState;

class World
{
  public:
    explicit World(ApplicationState& applicationState);

    void RegisterAll();

    ECS::ECSCoordinator& GetECS();

  private:
    ApplicationState&                    m_applicationState;
    std::unique_ptr<ECS::ECSCoordinator> m_ecsCoordinator;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_WORLD_HPP
