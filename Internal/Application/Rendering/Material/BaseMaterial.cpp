//
// Created by wpsimon09 on 25/03/25.
//

#include "BaseMaterial.hpp"

namespace ApplicationCore {

    BaseMaterial::BaseMaterial(std::shared_ptr<VulkanUtils::VEffect> effect):m_initialEffect(effect), m_materialEffect(effect)
    {
        ID = MaterialIndexCounter++;
    }

    void BaseMaterial::ChangeEffect(std::shared_ptr<VulkanUtils::VEffect> newEffect)
    {
        m_materialEffect = newEffect;
    }

} // ApplicationCore