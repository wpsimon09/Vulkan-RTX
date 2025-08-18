//
// Created by wpsimon09 on 25/03/25.
//

#include "BaseMaterial.hpp"

namespace ApplicationCore {

BaseMaterial::BaseMaterial(Renderer::EForwardRenderEffects effect): m_defaultMaterialEffect(effect), m_materialEffectType(effect) {
    ID = MaterialIndexCounter++;
}

void BaseMaterial::ResetEffect()
{
    m_defaultMaterialEffect ;
}

void BaseMaterial::SetMaterialname(std::string newName)
{
    m_materialName = newName;
}
uint32_t BaseMaterial::GetSceneIndex() { return m_sceneIndex; }

void     BaseMaterial::SetSceneIndex(uint32_t newIndex) {m_sceneIndex = newIndex;
}
void                            BaseMaterial::SetMaterialEffect(Renderer::EForwardRenderEffects newEffect) {m_materialEffectType = newEffect;}
Renderer::EForwardRenderEffects BaseMaterial::GetMatearialEffect() { return m_materialEffectType;}

}  // namespace ApplicationCore