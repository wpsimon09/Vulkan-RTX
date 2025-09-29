#ifndef PARAMETERSTRUCTS_HPP

#define PARAMETERSTRUCTS_HPP

#include "glm/glm.hpp"
#include <glm/fwd.hpp>

struct BilaterialFilterParameters
{
    float sigma  = 0.2f;
    float BSigma = 1.0f;
    float width;
    float height;
};

struct AoOcclusionParameters
{
    float radius      = 1.0;
    float sampleCount = 2.0;
    float strenght    = 4.0;
};

struct ToneMappingParameters
{
    float exposure     = 1.0;
    float gamma        = 1.0;
    float isRayTracing = 0.0;
    float curve        = 0.0;
};

struct LensFlareParameters
{
    float lensFlareStrength = 2.0f;
    float f1Strength        = 2.0f;
    float f2Strength        = 2.0f;
    float f3Strength        = 1.0f;
};

struct LuminanceHistogramParameters
{
    float width;
    float height;
    float minLogLuminance          = -10;
    float oneOverLogLuminanceRange = 0.0833;  // 2 - (-10) = 12; 1 / 12 = 0.0833
    float maxLogLuminance          = 2.0;
    float logRange                 = 12;
    float CalculateLuminanceRange()
    {
        float logRange           = maxLogLuminance - minLogLuminance;
        oneOverLogLuminanceRange = 1.0 / logRange;
        this->logRange           = logRange;
        return logRange;
    }
};

struct LuminanceHistogramAverageParameters
{
    float pixelCount;
    float minLogLuminance;
    float logLuminanceRange;
    float timeDelta;
    float tau = 1.1;
};

struct FogVolumeParameters
{
    float sigma_a{0.0001};  // not alterable through slider
    float sigma_s{0.0001};  // not alterable through slider
    float rayDistance{900.0f};
    float raySteps{4.0};

    glm::vec4 fogColour{0.0f};  // xyz - colour, w - density ,

    float heightFallOff{1};
    int   rayMarched      = false;
    float asymmetryFactor = {0.0f};
    float fogHeight       = {1.0f};
};

struct BloomUpSampleParams
{
    glm::vec4 src_xy_dst_xy;
    float     filterRadius;
    float     srcImage;
    float     dstImage;
    float     pad;
};

struct BloomDownSampleParams
{
    glm::vec4 src_xy_dst_xy;
    alignas(4) int srcImage;
    alignas(4) int dstImage;
};

struct BloomSettings
{
    glm::vec4 src_xy_dst_xy  = {};
    float     bloomStrenght  = 0.2;
    int       dirtTexture    = 0;
    float     dirstIntensity = 2.0f;
    float     filterRadius   = 0.005;
};

struct AtmosphereParameters
{
    glm::vec4 groundAlbedo = {0.3, 0.3, 0.3, 6360};             // xyz: as-is, w: Radius bottom
    glm::vec4 rayleighScattering{5.802, 13.558, 33.1, 6460.0};  // xyz: wevelength dependant, w: Radius top
    // rayleightAbsorption is 0 in paper so not included here
    glm::vec4 mieScattering = {3.996, 3.996, 3.996, 8.0};  // xyz: as-is, w: Rayleigh density exp scale
    glm::vec4 mieAbsorption = {4.40, 4.40, 4.40, 1.2};     // xyz: as-is, w: Mie density exp scale
    glm::vec4 mieExtinction = {8.369, 8.369, 8.369, 0.8};  // xyz: as-is, w: Mie phase g
    glm::vec4 absorptionExtinction;                        // xyz: as-is, w: Absorption density 0 layer width

    // absorption_density_0_constant_term;
    // absorption_density_0_linear_term;
    // absorption_density_1_constant_term;
    // absorption_density_1_linear_term;
    glm::vec4 absorption_density_params_;
};

#endif
