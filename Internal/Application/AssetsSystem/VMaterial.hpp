//
// Created by simon on 18/01/2026.
//

#ifndef VULKAN_RTX_VMATERIAL_HPP
#define VULKAN_RTX_VMATERIAL_HPP
#include "VAsset2.hpp"

namespace ApplicationCore {

class VMaterial : public VAsset2
{
  public:
    bool Save(std::filesystem::path& path) override;
};

}  // namespace ApplicationCore

#endif  //VULKAN_RTX_VMATERIAL_HPP
