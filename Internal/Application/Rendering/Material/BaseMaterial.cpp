//
// Created by wpsimon09 on 25/03/25.
//

#include "BaseMaterial.hpp"

namespace ApplicationCore {

BaseMaterial::BaseMaterial(uint32_t effectID)
    : m_defaultMaterialEffect(effectID)
    , m_materialEffectType(effectID)
{
    ID = MaterialIndexCounter++;
}

void BaseMaterial::ResetEffect()
{
    m_defaultMaterialEffect;
}

void BaseMaterial::SetMaterialname(std::string newName)
{
    m_materialName = newName;
}
uint32_t BaseMaterial::GetSceneIndex()
{
    return m_sceneIndex;
}

void BaseMaterial::SetSceneIndex(uint32_t newIndex)
{
    m_sceneIndex = newIndex;
}
void BaseMaterial::SetMaterialEffect(uint32_t newEffect)
{
    m_materialEffectType = newEffect;
}

uint32_t BaseMaterial::GetMatearialEffect()
{
    return m_materialEffectType;
}

}  // namespace ApplicationCore