//
// Created by wpsimon09 on 14/07/25.
//

#ifndef APPLICATIONSTATE_HPP
#define APPLICATIONSTATE_HPP


#include "Application/Enums/ClientEnums.hpp"
#include "Application/Structs/ParameterStructs.hpp"
#include "Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp"
struct GlobalRenderingInfo;
struct SceneUpdateFlags;
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

    SceneUpdateFlags& GetSceneUpdateFlags();
    void              pSetSceneUpdateFlags(SceneUpdateFlags* sceneUpdateFlags);

    GlobalRenderingInfo& GetGlobalRenderingInfo();
    void                 pSetGlobalRenderingInfo(GlobalRenderingInfo* pGlobalRenderingInfo);

    BilaterialFilterParameters& GetBilateralFilaterParameters();
    AoOcclusionParameters&      GetAoOcclusionParameters();
    ToneMappingParameters&      GetToneMappingParameters();
    LensFlareParameters&        GetLensFlareParameters();

    bool& IsWindowResized();
    void  SetIsWindowResized(bool windowResized);

  public:
    EDebugRendering    m_rendererOutput   = EDebugRendering::Lit;
    EDebugRenderingRTX m_rtxRenderOutput  = EDebugRenderingRTX::PathTraced;
    bool               m_accumulateFrames = true;
    bool               m_ambientOcclusion = false;


  private:
    LightStructs::SceneLightInfo* m_sceneLight          = nullptr;  // instantiated in Scene.hpp
    SceneData*                    m_sceneData           = nullptr;  // instantiated in Scene.hpp
    SceneUpdateFlags*             m_sceneUpdateFlags    = nullptr;  // instantiated in Scene.hpp
    GlobalRenderingInfo*          m_globalRenderingInfo = nullptr;  // instantiated in Client.hpp

    BilaterialFilterParameters m_bilaterialFilaterParameters;
    AoOcclusionParameters      m_aoOcclusionParameters;
    ToneMappingParameters      m_toneMappingParameters;
    LensFlareParameters        m_lensFlareParameters;

    bool m_windowResized = false;
};

}  // namespace ApplicationCore

#endif  //APPLICATIONSTATE_HPP
