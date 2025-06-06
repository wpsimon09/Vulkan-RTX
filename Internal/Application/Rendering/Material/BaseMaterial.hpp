//
// Created by wpsimon09 on 25/03/25.
//

#ifndef BASEMATERIAL_HPP
#define BASEMATERIAL_HPP
#include "Vulkan/Global/GlobalVulkanEnums.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace VulkanUtils {
class VRasterEffect;
class VUniformBufferManager;
};  // namespace VulkanUtils

namespace ApplicationCore {
class VTextureAsset;

inline int MaterialIndexCounter = 0;

/**
 * This class is not used in rendering, only Material or SkyBoxMaterial can be used
 */
class BaseMaterial
{
  public:
    BaseMaterial(std::shared_ptr<VulkanUtils::VRasterEffect> effect);

    bool&                                        IsTransparent() { return m_transparent; }
    void                                         SetTransparent(bool value) { m_transparent = value; }
    bool                                         IsSavable() const { return m_savable; }
    void                                         SetSavable(bool savable) { m_savable = savable; }
    int                                          GetID() { return ID; }
    virtual void                                 ChangeEffect(std::shared_ptr<VulkanUtils::VRasterEffect> newEffect);
    std::string&                                 GetMaterialName() { return m_materialName; }
    std::shared_ptr<VulkanUtils::VRasterEffect>& GetEffect();
    void                                         ResetEffect();
    void                                         SetMaterialname(std::string newName);
    uint32_t                                     GetSceneIndex();
    void                                         SetSceneIndex(uint32_t newIndex);
    virtual std::vector<std::shared_ptr<VTextureAsset>> EnumarateTexture() = 0;

  protected:
    bool                                        m_transparent = false;
    bool                                        m_savable     = false;
    int                                         ID;
    std::shared_ptr<VulkanUtils::VRasterEffect> m_materialEffect;
    std::shared_ptr<VulkanUtils::VRasterEffect> m_initialEffect;
    std::string                                 m_materialName;

    uint32_t m_sceneIndex = 0;


  private:
    friend bool operator==(const BaseMaterial& lhs, const BaseMaterial& rhs) { return lhs.ID == rhs.ID; }

    friend bool operator!=(const BaseMaterial& lhs, const BaseMaterial& rhs) { return !(lhs == rhs); }
};

}  // namespace ApplicationCore

#endif  //BASEMATERIAL_HPP
