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

/*
 A density profile layer describes how the concentration of a scattering or
 absorbing species varies with altitude. Each layer is valid for a vertical
 interval of size `width`, and within that interval the density is defined as:

     density(h) = exp_term * exp(exp_scale * h)
                + linear_term * h
                + constant_term

 where h is the altitude above the bottom of the layer. The resulting value
 is clamped to [0,1]. By combining multiple layers, we can approximate real
 atmospheric profiles such as:

   - Rayleigh scattering (air molecules) → mostly exponential falloff
   - Mie scattering (aerosols) → different exponential scale
   - Ozone absorption → can be shaped with linear + constant terms

 These profiles are later integrated along view/light paths to compute
 transmittance, scattering, and absorption in the atmosphere.
*/
 struct DensityProfileLayer{
    float width;
    float exponentialTerm;
    float expScale;
    float linearTerm;
    float constantTerm;
}

//=========================================================================================================

/*
Converts from parametrized domain (mu and r) to the texture uv domain
    - for  x = v || u
*/
float GetTextureCoordFromUnitRange(float x, int textureSize){
    return 0.5 / float(textureSize) + x * (1.0 - 1.0/float(textureSize));
}
/*
Converts from to the texture domain (u, v) to parametrized domain (mu and r) 
    - for u
*/
float GetUnitRangeFromTextureCoord(float u, int textureSize){
    return (u - 0.5 / float(textureSize))/ (1.0 - 1.0 /float(textureSize));
}



float DistanceToTopAtmosphereBoundary(AtmosphereParamsGPU atmosphere,float r, float mu){
    float discriminatn = r * r * (mu * mu - 1.0) + atmosphere.radiusTop * atmosphere.radiusTop;
    return max((-r * mu + sqrt(discriminatn)),0.0);
}

float DistanceToBottomAtmosphereBoundary(AtmosphereParamsGPU atmosphere, float r, float mu){
    float discriminant = r * r * (mu * mu - 1.0) + atmosphere.radiusBottom * atmosphere.radiusBottom;
    return max(( -r * mu - sqrt(discriminant)), 0.0);
}

float GetLayerDensity(DensityProfileLayer)

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
    float rho = sqrt(r * r - atmosphere.radiusBottom * atmosphere.radiusBottom);

    // distance to the top of the atmosphere verticaly 
    float d = DistanceToTopAtmosphereBoundary(atmosphere,r, mu);
    float d_min = atmosphere.radiusTop - r;
    float d_max = rho + H;
    
    // normalize to the 0, 1
    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;
    return float2(GetTextureCoordFromUnitRange(x_mu, (int)atmosphere.transmitanceTextureWidth),
                  GetTextureCoordFromUnitRange(x_r, (int)atmosphere.transmitanceTextureHeight));
}

/*
Converts texture coordinate 
*/
void GetRMuFromTransmitanceTextureUv(AtmosphereParamsGPU atmosphere, float2 uv, out float r, out float mu){
    float x_mu = GetUnitRangeFromTextureCoord(uv.x,(int)atmosphere.transmitanceTextureWidth);
    float x_r = GetUnitRangeFromTextureCoord(uv.y, (int)atmosphere.transmitanceTextureHeight);
    
    // Distance to the top atmosphere boundary
    float H = sqrt(atmosphere.radiusTop * atmosphere.radiusTop - atmosphere.radiusBottom * atmosphere.radiusBottom);

    // distance to the horizon
    float rho = H * x_r;

    r = sqrt(rho * rho + atmosphere.radiusBottom * atmosphere.radiusBottom);

    float d_min = atmosphere.radiusTop - r;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    
    mu = d == 0.0 * 1 ? float(1.0) : (H * H - rho * rho - d * d) / (2.0 * r * d);
    mu = clamp(mu, float(-1.0), float(1.0));
}


//=============================================================
// Transmitance pre-computation

float3 ComputeTransmittanceToTopAtmosphereBoundaryTexture(AtmosphereParamsGPU atmosphere, float2 pixelCoord){
    const float2 transmitanceTextureSize = float2(atmosphere.transmitanceTextureWidth, atmosphere.transmitanceTextureHeight);
    
    float r;
    float mu;
    GetRMuFromTransmitanceTextureUv(atmosphere, pixelCoord / transmitanceTextureSize, r, mu);
    return ComputeTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);
}

float3 ComputeTransmittanceToTopAtmosphereBoundary(AtmosphereParamsGPU atmoshere, float r, float mu){
    return exp(-(
        atmoshere.rayLeightScattering * ComputeOpticalLengthToTopAtmosphereBoundary(atmo)
    ))
}