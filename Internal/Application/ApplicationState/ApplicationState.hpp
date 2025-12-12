//
// Created by wpsimon09 on 14/07/25.
//

#ifndef APPLICATIONSTATE_HPP
#define APPLICATIONSTATE_HPP


#include "Application/Enums/ClientEnums.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp"
struct GlobalRenderingInfo;
struct SceneUpdateContext;
namespace LightStructs {
struct SceneLightInfo;
}
namespace ApplicationCore {
struct SceneData;

/**
 * Class that holds data for renderer and  Editor with various information about current state of the non-rendering code
 */
class ApplicationState
{
  public:
    ApplicationState();

    void Update();

    void Reset();

    LightStructs::SceneLightInfo& GetSceneLightInfo();
    void                          pSetSceneLightInfo(LightStructs::SceneLightInfo* pSceneLight);

    SceneData& GetSceneData();
    void       pSetSceneData(SceneData* pSceneData);

    SceneUpdateContext& GetSceneUpdateFlags();
    void                pSetSceneUpdateFlags(SceneUpdateContext* sceneUpdateFlags);

    GlobalRenderingInfo& GetGlobalRenderingInfo();
    void                 pSetGlobalRenderingInfo(GlobalRenderingInfo* pGlobalRenderingInfo);

    FogVolumeParameters* GetFogVolumeParameters();
    void                 pSetFogVolumeParameters(FogVolumeParameters* pFogVolumeParameters);

    AtmosphereParameters* GetAtmosphereParameters();
    void                  pSetAtmosphereParameters(AtmosphereParameters* pAtmosphereParams);

    BilaterialFilterParameters&          GetBilateralFilaterParameters();
    AoOcclusionParameters&               GetAoOcclusionParameters();
    ToneMappingParameters&               GetToneMappingParameters();
    LensFlareParameters&                 GetLensFlareParameters();
    LuminanceHistogramParameters&        GetLuminanceHistogramParameters();
    LuminanceHistogramAverageParameters& GetLuminanceAverageParameters();
    BloomSettings&                       GetBloomSettings();


    bool& IsWindowResized();
    void  SetIsWindowResized(bool windowResized);

  public:
    EDebugRendering    m_rendererOutput       = EDebugRendering::Lit;
    EDebugRenderingRTX m_rtxRenderOutput      = EDebugRenderingRTX::PathTraced;
    bool               m_accumulateFrames     = true;
    bool               m_ambientOcclusion     = false;
    bool               m_enablePostProcessing = true;
    bool               m_denoise              = true;
    bool m_composite = true;  // use separate draw pass to composite the visibility buffer and any other effects that might contribute with the rest of the scene
    bool m_rayTracedReflections = true;

  private:
    LightStructs::SceneLightInfo* m_sceneLight          = nullptr;  // instantiated in Scene.hpp
    SceneData*                    m_sceneData           = nullptr;  // instantiated in Scene.hpp
    SceneUpdateContext*           m_sceneUpdateFlags    = nullptr;  // instantiated in Scene.hpp
    GlobalRenderingInfo*          m_globalRenderingInfo = nullptr;  // instantiated in Client.hpp
    FogVolumeParameters*          m_fogVolumeParameters = nullptr;
    AtmosphereParameters*         m_atmosphereParams    = nullptr;

    BilaterialFilterParameters          m_bilaterialFilaterParameters;
    AoOcclusionParameters               m_aoOcclusionParameters;
    ToneMappingParameters               m_toneMappingParameters;
    LensFlareParameters                 m_lensFlareParameters;
    LuminanceHistogramParameters        m_luminanceHistrogramParameters;
    LuminanceHistogramAverageParameters m_luminanceAverageParameters;
    BloomSettings                       m_bloomSettings;


    bool m_windowResized = false;
};

}  // namespace ApplicationCore

#endif  //APPLICATIONSTATE_HPP
