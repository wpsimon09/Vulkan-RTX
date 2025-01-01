//
// Created by wpsimon09 on 21/12/24.
//

#include "IUserInterfaceElement.hpp"

void VEditor::IUserInterfaceElement::Render()
{
    for (auto &uiChild : m_uiChildren)
    {
        uiChild->Render();
    }
}

void VEditor::IUserInterfaceElement::Update()
{
    for (auto &uiChild : m_uiChildren)
    {
        uiChild->Update();
    }
}
