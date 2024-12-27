//
// Created by wpsimon09 on 21/12/24.
//

#include "ViewPort.hpp"

#include <imgui.h>

VEditor::ViewPort::ViewPort(ViewPortContext& viewPortContext): m_viewPortContext(viewPortContext), IUserInterfaceElement{}
{
}

void VEditor::ViewPort::Init()
{
    IUserInterfaceElement::Init();
}

void VEditor::ViewPort::Resize(int newWidth, int newHeight)
{
}


