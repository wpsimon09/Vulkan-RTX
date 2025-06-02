//
// Created by wpsimon09 on 25/03/25.
//

#include "BaseMaterial.hpp"

namespace ApplicationCore {

BaseMaterial::BaseMaterial(std::shared_ptr<VulkanUtils::VRasterEffect> effect)
    : m_initialEffect(effect)
    , m_materialEffect(effect)
{
    ID = MaterialIndexCounter++;
}

void BaseMaterial::ChangeEffect(std::shared_ptr<VulkanUtils::VRasterEffect> newEffect)
{
    m_materialEffect = newEffect;
}

std::shared_ptr<VulkanUtils::VRasterEffect>& BaseMaterial::GetEffect()
{
    return m_materialEffect;
}

void BaseMaterial::ResetEffect()
{
    m_materialEffect = m_initialEffect;
}

void BaseMaterial::SetMaterialname(std::string newName)
{
    m_materialName = newName;
}
uint32_t BaseMaterial::GetSceneIndex() { return m_sceneIndex; }

void     BaseMaterial::SetSceneIndex(uint32_t newIndex) {m_sceneIndex = newIndex;}

}  // namespace ApplicationCore