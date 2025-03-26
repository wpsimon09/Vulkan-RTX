//
// Created by wpsimon09 on 25/03/25.
//

#ifndef BASEMATERIAL_HPP
#define BASEMATERIAL_HPP
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace VulkanUtils
{
    class VEffect;
}

namespace ApplicationCore {

inline int MaterialIndexCounter = 0;

/**
 * This class is not used in rendering, only Material or SkyBoxMaterial can be used
 */
class BaseMaterial {
public:
    BaseMaterial(std::shared_ptr<VulkanUtils::VEffect> effect);

    bool&                                                   IsTransparent() { return m_transparent; }
    void                                                    SetTransparent(bool value) { m_transparent = value; }
    bool                                                    IsSavable() const {return m_savable;}
    void                                                    SetSavable(bool savable) {m_savable = savable;}
    int                                                     GetID() {return ID;}
    virtual void                                            ChangeEffect(std::shared_ptr<VulkanUtils::VEffect> newEffect);
    std::string&                                            GetMaterialName() { return m_materialName; };
    void                                                    SetMaterialname(std::string newName);
    void                                                    UpdateGPU(vk::CommandBuffer& cmdBuferr) = 0;                         


protected:
    bool m_transparent = false;
    bool m_savable = false;
    int ID;
    std::shared_ptr<VulkanUtils::VEffect> m_materialEffect;
    std::shared_ptr<VulkanUtils::VEffect> m_initialEffect;
    std::string m_materialName;



private:

    friend bool operator==(const BaseMaterial& lhs, const BaseMaterial& rhs)
    {
        return lhs.ID == rhs.ID;
    }

    friend bool operator!=(const BaseMaterial& lhs, const BaseMaterial& rhs)
    {
        return !(lhs == rhs);
    }

};

} // ApplicationCore

#endif //BASEMATERIAL_HPP
