//
// Created by wpsimon09 on 14/07/25.
//

#include "ApplicationState.hpp"

namespace ApplicationCore {
ApplicationState::ApplicationState() {}

void ApplicationState::Update() {}

void ApplicationState::Reset()
{
    m_windowResized = false;
}
}  // namespace ApplicationCore