//
// Created by wpsimon09 on 21/12/24.
//

#include "IUserInterfaceElement.hpp"

void VEditor::IUserInterfaceElement::Init()
{
    for (auto &uiChild : m_uiChildren)
    {
        uiChild->Init();
    }
}
