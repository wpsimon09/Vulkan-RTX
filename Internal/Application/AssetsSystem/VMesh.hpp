//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VMESH_HPP
#define VULKAN_RTX_VMESH_HPP
#include "VAsset2.hpp"

namespace ApplicationCore {

class VMesh : public VAsset2
{
  public:
    bool Save(std::filesystem::path& path) override;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VMESH_HPP
