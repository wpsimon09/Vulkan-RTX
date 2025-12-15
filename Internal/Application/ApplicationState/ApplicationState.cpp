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
    , m_luminanceHistrogramParameters{}
{
}

void ApplicationState::Update() {}

void ApplicationState::Reset()
{
    m_windowResized = false;
    //m_globalRenderingInfo->accountForAtmosphereTransmitance = false;
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

SceneUpdateContext& ApplicationState::GetSceneUpdateFlags()
{
    return *m_sceneUpdateFlags;
}
void ApplicationState::pSetSceneUpdateFlags(SceneUpdateContext* sceneUpdateFlags)
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
FogVolumeParameters* ApplicationState::GetFogVolumeParameters()
{
    return m_fogVolumeParameters;
}
void ApplicationState::pSetFogVolumeParameters(FogVolumeParameters* pFogVolumeParameters)
{
    m_fogVolumeParameters = pFogVolumeParameters;
}

BloomSettings& ApplicationState::GetBloomSettings()
{
    return m_bloomSettings;
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

LuminanceHistogramParameters& ApplicationState::GetLuminanceHistogramParameters()
{
    return m_luminanceHistrogramParameters;
}

LuminanceHistogramAverageParameters& ApplicationState::GetLuminanceAverageParameters()
{
    return m_luminanceAverageParameters;
}

AtmosphereParameters* ApplicationState::GetAtmosphereParameters()
{
    return m_atmosphereParams;
}

void ApplicationState::pSetAtmosphereParameters(AtmosphereParameters* pAtmosphereParams)
{
    m_atmosphereParams = pAtmosphereParams;
    // TODO: for now i dont know of better place to tell renderer that it should attenuate sun based on the zenith angle
    m_globalRenderingInfo->accountForAtmosphereTransmitance = static_cast<bool>(pAtmosphereParams->booleans.z);
    // camera position in global data is going to be bottom sphere radius
    m_globalRenderingInfo->atmosphereParams.x = pAtmosphereParams->groundAlbedo.w;
    m_globalRenderingInfo->atmosphereParams.y = pAtmosphereParams->rayleighScattering.w;

    // for the new packed type
    m_globalRenderingData2->atmosphereParams.x = pAtmosphereParams->groundAlbedo.w;        // top radius
    m_globalRenderingData2->atmosphereParams.y = pAtmosphereParams->rayleighScattering.w;  // botom radius
    m_globalRenderingData2->atmosphereParams.z = static_cast<bool>(pAtmosphereParams->booleans.z);  // account for transmitance in the light calculations
}

void ApplicationState::pSetGlobalRenderingInfoData2(GlobalRenderingInfo2* pGlobalRenderingInfo)
{
    m_globalRenderingData2 = pGlobalRenderingInfo;
}

GlobalRenderingInfo2& ApplicationState::GetGlobalRenderingInfo2()
{
    return *m_globalRenderingData2;
}


}  // namespace ApplicationCore