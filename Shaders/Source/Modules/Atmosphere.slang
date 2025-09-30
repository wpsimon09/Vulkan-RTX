//========================================================================
// Every parameter related to the atmosphere will live here
struct AtmosphereParamsCPU{
    float4 groundAlbedo; //xyz - colour, w: Radius bottom
    float4 rayleighScattering; //xyz - wavelenght dependad, w :radius top
    float4 mieScattering;  //xyz: as-is, w:Reayleigh density exp scale 
    float4 mieAbsorption;//xyz: as-is, w: Mie density exp scale
    float4 mieExctinction;//xyz: as-is, w: Mie phase G
    float4 absorptionExctinction; //xyz: as-is, w: Absorption denssity 0 layer width

    AtmosphereParamsGPU Parse(){
         AtmosphereParamsGPU gpu;

    gpu.groundAlbedo = groundAlbedo.xyz;
    gpu.radiusBottom = groundAlbedo.w;

    gpu.rayLeightScattering = rayleighScattering.xyz;
    gpu.radiusTop = rayleighScattering.w;

    gpu.mieScattering = mieScattering.xyz;
    gpu.reayleghtDensityExpScale = mieScattering.w;

    gpu.mieAbsorption = mieAbsorption.xyz;
    gpu.mieDensityExpScale = mieAbsorption.w;

    gpu.mieExctinction = mieExctinction.xyz;
    gpu.miePhaseG = mieExctinction.w;

    gpu.absorptionExctinction = absorptionExctinction.xyz;
    gpu.absorptionDensityZeroLayerWidth = absorptionExctinction.w;

    // Typically you’ll also assign LUT sizes here (hardcoded or passed in).
    gpu.transmitanceTextureWidth  = 256.0f; // or your chosen value
    gpu.transmitanceTextureHeight = 64.0f;  // or your chosen value

    return gpu;
    }
}

struct AtmosphereParamsGPU{
    float3 groundAlbedo;
    float radiusBottom;

    float3 rayLeightScattering;
    float radiusTop;

    float3 mieScattering;
    float reayleghtDensityExpScale;
    
    float3 mieAbsorption;
    float mieDensityExpScale;
    
    float3 mieExctinction;
    float miePhaseG;
    
    float4 absorptionExctinction;
    float absorptionDensityZeroLayerWidth;

    float transmitanceTextureWidth;
    float transmitanceTextureHeight;
}

//=========================================================================================================

float DistanceToTopAtmosphereBoundary(AtmosphereParamsGPU atmosphere,float r, float mu){
    float discriminatn = r * r * (mu * mu - 1.0) + atmosphere.radiusTop * atmosphere.radiusTop;
    return clamp(-r * mu + sqrt(discriminatn));
}


/*
Converts the azimuth (mu) and radius (r) to the the texture coordinates so that we can write to the texture 
*/
float2 GetTransmittanceTextureUvFromRMu(AtmosphereParamsGPU atmosphere, float r, float mu){
    //if(!(r >= atmosphere.radiusBottom && r <= atmosphere.radiusTop)){return {-1,-1};}
    //if(!(mu >= -1.0 && mu <= 1.0)) {}

    //compute how far a horizontal ray at sea level goes before it escapes the atmosphere.
    // see figure 2 the big H
    float H = sqrt(atmosphere.radiusTop *atmosphere.radiusTop  - atmosphere.radiusBottom * atmosphere.radiusBottom);

    // distance to the horizon, see figure 2 in 
    //https://ebruneton.github.io/precomputed_atmospheric_scattering/atmosphere/functions.glsl.html#transmittance_computation
    float rho = sqrt(r * r - atmosphere.radiusBottom * atmosphere.radiusBottom)

    // distance to the top of the atmosphere verticaly 
    float d = DistanceToTopAtmosphereBoundary(atmosphere,r, mu);
}