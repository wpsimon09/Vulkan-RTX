//
// Created by wpsimon09 on 14/07/25.
//

#include "ApplicationState.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp"

namespace ApplicationCore {
ApplicationState::ApplicationState()
    : m_bilaterialFilaterParameters{}
    , m_aoOcclusionParameters{}
    , m_toneMappingParameters{}
{
}

void ApplicationState::Update() {}

void ApplicationState::Reset()
{
    m_windowResized = false;
}
LightStructs::SceneLightInfo& ApplicationState::GetSceneLightInfo()
{
    return *m_sceneLight;
}
void ApplicationState::pSetSceneLightInfo(LightStructs::SceneLightInfo* pSceneLight)
{
    m_sceneLight = pSceneLight;
}

SceneData& ApplicationState::GetSceneData()
{
    return *m_sceneData;
}
void ApplicationState::pSetSceneData(SceneData* pSceneData)
{
    m_sceneData = pSceneData;
}

SceneUpdateFlags& ApplicationState::GetSceneUpdateFlags()
{
    return *m_sceneUpdateFlags;
}
void ApplicationState::pSetSceneUpdateFlags(SceneUpdateFlags* sceneUpdateFlags)
{
    m_sceneUpdateFlags = sceneUpdateFlags;
}

GlobalRenderingInfo& ApplicationState::GetGlobalRenderingInfo()
{
    return *m_globalRenderingInfo;
}
void ApplicationState::pSetGlobalRenderingInfo(GlobalRenderingInfo* pGlobalRenderingInfo)
{
    m_globalRenderingInfo = pGlobalRenderingInfo;
}


bool& ApplicationState::IsWindowResized()
{
    return m_windowResized;
}
void ApplicationState::SetIsWindowResized(bool windowResized)
{
    m_windowResized = windowResized;
}
BilaterialFilterParameters& ApplicationState::GetBilateralFilaterParameters()
{
    return m_bilaterialFilaterParameters;
}

AoOcclusionParameters& ApplicationState::GetAoOcclusionParameters()
{
    return m_aoOcclusionParameters;
}

ToneMappingParameters& ApplicationState::GetToneMappingParameters()
{
    return m_toneMappingParameters;
}

LensFlareParameters& ApplicationState::GetLensFlareParameters()
{
    return m_lensFlareParameters;
}

}  // namespace ApplicationCore