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
LightStructs::SceneLightInfo& ApplicationState::GetSceneLightInfo() { return *m_sceneLight;}
void                          ApplicationState::pSetSceneLightInfo(LightStructs::SceneLightInfo* pSceneLight) {m_sceneLight = pSceneLight}
SceneData&                    ApplicationState::GetSceneData() {return *m_sceneData;}
void                          ApplicationState::pSetSceneData(SceneData* pSceneData) {m_sceneData = pSceneData;}
SceneUpdateFlags&             ApplicationState::GetSceneUpdateFlags() { return *m_sceneUpdateFlags;}
void                          ApplicationState::pSetSceneUpdateFlags(SceneUpdateFlags* sceneUpdateFlags) {}
GlobalRenderingInfo&          ApplicationState::GetGlobalRenderingInfo() {}
void                          ApplicationState::pSetGlobalRenderingInfo(GlobalRenderingInfo* m_global_rendering_info) {}
bool&                         ApplicationState::IsWindowResized() {}
void                          ApplicationState::SetIsWindowResized(bool windowResized) {}

}  // namespace ApplicationCore