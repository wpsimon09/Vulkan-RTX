module Atmosphere;

import AtmosphereParameters;

//=========================================================================================================

static const float m = 1.0;
static const float m2 = m * m;

float ClampCosine(float mu) {
  return clamp(mu, float(-1.0), float(1.0));
}

float ClampDistance(float d) {
  return max(d, 0.0 * m);
}

float ClampRadius(AtmosphereParamsGPU atmosphere, float r) {
  return clamp(r, atmosphere.radiusBottom, atmosphere.radiusTop);
}

float SafeSqrt(float a) {
  return sqrt(max(a, 0.0 * (m2)));
}


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
    return ClampDistance(-r * mu + SafeSqrt(discriminatn));
}

float DistanceToBottomAtmosphereBoundary(AtmosphereParamsGPU atmosphere, float r, float mu){
    float discriminant = r * r * (mu * mu - 1.0) + atmosphere.radiusBottom * atmosphere.radiusBottom;
    return ClampDistance(-r * mu - SafeSqrt(discriminant));
}

float GetLayerDensity(DensityProfileLayer layer, float altitude){
    float density = layer.exponentialTerm * exp(layer.expScale * altitude) + layer.linearTerm * altitude + layer.constantTerm;
    return clamp(density, 0.0, 1.0);
}

float GetProfileDensity(DensityProfile profile, float altitude){
    return altitude < profile.layers[0].width ?
        GetLayerDensity(profile.layers[0], altitude) :
        GetLayerDensity(profile.layers[1], altitude);
}


/*
Converts the azimuth (mu) and radius (r) to the the texture coordinates so that we can write to the texture 
*/
float2 GetTransmittanceTextureUvFromRMu(AtmosphereParamsGPU atmosphere, float r, float mu){
    //if(!(r >= atmosphere.radiusBottom && r <= atmosphere.radiusTop)){return {-1,-1};}
    //if(!(mu >= -1.0 && mu <= 1.0)) {}

    //compute how far a horizontal ray at sea level goes before it escapes the atmosphere.
    // see figure 2 the big H
    float H = SafeSqrt(atmosphere.radiusTop *atmosphere.radiusTop  - atmosphere.radiusBottom * atmosphere.radiusBottom);

    // distance to the horizon, see figure 2 in 
    //https://ebruneton.github.io/precomputed_atmospheric_scattering/atmosphere/functions.glsl.html#transmittance_computation
    float rho = SafeSqrt(r * r - atmosphere.radiusBottom * atmosphere.radiusBottom);

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
public void GetRMuFromTransmitanceTextureUv(AtmosphereParamsGPU atmosphere, float2 uv, out float r, out float mu){
    float x_mu = GetUnitRangeFromTextureCoord(uv.x,(int)atmosphere.transmitanceTextureWidth);
    float x_r = GetUnitRangeFromTextureCoord(uv.y, (int)atmosphere.transmitanceTextureHeight);
    
    // Distance to the top atmosphere boundary
    float H = SafeSqrt(atmosphere.radiusTop * atmosphere.radiusTop - atmosphere.radiusBottom * atmosphere.radiusBottom);

    // distance to the horizon
    float rho = H * x_r;

    r = SafeSqrt(rho * rho + atmosphere.radiusBottom * atmosphere.radiusBottom);

    float d_min = atmosphere.radiusTop - r;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    
    mu = d == 0.0 * 1 ? float(1.0) : (H * H - rho * rho - d * d) / (2.0 * r * d);
    mu = clamp(mu, float(-1.0), float(1.0));
}


//=============================================================
// Transmitance pre-computation

public float3 ComputeTransmittanceToTopAtmosphereBoundaryTexture(AtmosphereParamsGPU atmosphere, float2 uv){
    const float2 transmitanceTextureSize = float2(atmosphere.transmitanceTextureWidth, atmosphere.transmitanceTextureHeight);
    
    float r;
    float mu;
    GetRMuFromTransmitanceTextureUv(atmosphere, uv, r, mu);
    return ComputeTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);
}

float ComputeOpticalLengthToTopAtmosphereBoundary(AtmosphereParamsGPU atmosphere, DensityProfile profle, float r, float mu){
    const int SAMPLE_COUNT  = 500;
    float dx = DistanceToTopAtmosphereBoundary(atmosphere, r, mu) / SAMPLE_COUNT;

    //integrate the transmitance for one participating medium
    float result = 0.0 * m;
    for (int i = 0; i<= SAMPLE_COUNT; ++i){
        float d_i = float(i) * dx;
        
        //distance between planet center and current sample point 
        float r_i = SafeSqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);

        float y_i = GetProfileDensity(profle, r_i - atmosphere.radiusBottom);

        float weight_i = i == 0 || i == SAMPLE_COUNT ? 0.5 : 1.0;

        result += y_i * weight_i * dx;
    }

    return result;
}

// Computes the total transmittance from position r along direction mu
// by integrating optical depths of all atmospheric components (Rayleigh scattering,
// Mie scattering, and ozone absorption) up to the top of the atmosphere.

float3 ComputeTransmittanceToTopAtmosphereBoundary(AtmosphereParamsGPU atmosphere, float r, float mu){

    float3 rayLeightTransmitance, mieTransmitance, ozoneTransmitance;

    rayLeightTransmitance = atmosphere.rayLeightScattering * ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.rayleightDensity, r, mu);
    mieTransmitance = atmosphere.mieExtinction * ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.mieDensity, r, mu);
    ozoneTransmitance = atmosphere.absorptionExctinction * ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.ozoneDensity, r, mu);
    //  + mieTransmitance + ozoneTransmitance
    return exp(-(rayLeightTransmitance + mieTransmitance + ozoneTransmitance));
}