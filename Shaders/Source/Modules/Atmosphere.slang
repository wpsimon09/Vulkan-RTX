module Atmosphere;

import AtmosphereParameters;
import Utils;
import Intersections;
//=========================================================================================================

public static const float PI = 3.1415926535897932384626433832795;


public float ClampRadius(AtmosphereParamsGPU atmosphere, float r) {
  return clamp(r, atmosphere.radiusBottom, atmosphere.radiusTop);
}


//==================================================================================================
//======================================= HELPER FUNCTIONS  ========================================
//==================================================================================================

public float2 UvToSkyViewLUTParams(float2 uv, float2 atmosPhereBoundaries, float2 skyViewDimenstions, float viewHeight){
    uv = float2(fromSubUvsToUnit(uv.x, skyViewDimenstions.x),
                fromSubUvsToUnit(uv.y, skyViewDimenstions.y));
    
    float beta = asin(atmosPhereBoundaries.x / viewHeight);
    float zenithHorizonAngle = PI - beta;

    float viewZenithAngle;
    float lightViewAngle;

    /* Non uniform mapping as described in the paper.
       Since there is low visual frequency through the map, we should 
       put more pixels towards horizon to get the best results 
    */
    if(uv.y < 0.5){
        float coord = 1.0 - (1.0 - 2.0 * uv.y) * (1.0 - 2.0 * uv.y);
        viewZenithAngle = zenithHorizonAngle * coord;
    }
    else{
        float coord = (uv.y * 2.0 -1.0) * (uv.y * 2.0 - 1.0);
        viewZenithAngle = zenithHorizonAngle + beta * coord;
    }
    lightViewAngle = (uv.x * uv.x) * PI;
    return float2(viewZenithAngle, lightViewAngle);
}

public float2 SkyViewLutParamsToUv(bool intersectsGround, float2 LUTParams, float viewHeight, float2 atmosBoundaries, float2 skyViewDimensions){
    float2 uv;
    float beta = asin(atmosBoundaries.x / viewHeight);
    float zeithHorizonAngle = PI - beta;

    if(!intersectsGround){
        float coord = LUTParams.x / zeithHorizonAngle;
        coord = (1.0 - SafeSqrt(1.0 - coord)) / 2.0;
        uv.y = coord;
    }
    else{
        float coord = (LUTParams.x - zeithHorizonAngle) / beta;
        coord = (SafeSqrt(coord) + 1.0) / 2.0;
        uv.y = coord;
    }
    uv.x = SafeSqrt(LUTParams.y / PI);
    uv = float2(
        fromUnitToSubUvs(uv.x, skyViewDimensions.x),
        fromUnitToSubUvs(uv.y, skyViewDimensions.y);
    )
    return uv;
}

public float fromSubUvsToUnit(float u, float resolution) {
	return (u - 0.5 / resolution) * (resolution / (resolution - 1.0)); 
}

public float fromUnitToSubUvs(float u, float resolution) {
	 return (u + 0.5f / resolution) * (resolution / (resolution + 1.0));
}


/*
Samples the medium extinction (i.e., absorption) for each component of the atmosphere.
Calculates how much light is absorbed by Mie particles, Rayleigh particles, and ozone at a given world position.
Returns the total extinction as the sum of all components.
*/
public float3 SampleMediumExctinction(AtmosphereParamsGPU atmosphere,float3 worldPosition){
    const float viewHeight = length(worldPosition) - atmosphere.radiusBottom;

    // get the denity of the medium in the given height 
    // mie 1.2/H , rayileigh 8.0/H
    const float densityMie = exp(atmosphere.mieDensity.layers[1].expScale * viewHeight);
    const float densityRei = exp(atmosphere.rayleightDensity.layers[1].expScale * viewHeight);
    const float densityOzo = atmosphere.getOzoneDensity(viewHeight); 

    // get exctincion coeficient of differnet particales in the atmosphere and scaled it based on the height 
    float3 mieExtinction = atmosphere.mieExtinction * densityMie;
    float3 rayExtinction = atmosphere.rayLeightScattering * densityRei;
    float3 ozoneExctinction = atmosphere.absorptionExctinction * densityOzo;

    return mieExtinction + rayExtinction + ozoneExctinction; // total atentuation of the energy 
}

/*
Samples the total scattering of light in the atmosphere, independent of direction.
Calculates the contribution from Mie and Rayleigh scattering at a given world position.
Ozone only absorbs light and does not contribute to scattering.
Returns the sum of all scattering components.

returns mie + phase + atmos, mie and phase are outputed separately in the parameters
*/
public float3 SampleMediumScattering(AtmosphereParamsGPU atmosphere, float3 worldPostion, out float3 mieScatter, out float3 rayScatter){
    const float viewHeight = length(worldPostion) - atmosphere.radiusBottom;

    // get the denity of the medium in the given height 
    // densities are from the paper
    //e^(-h / 1.2km)
    const float densityMie = exp(atmosphere.mieDensity.layers[1].expScale * viewHeight);
    //e^(-h / 8km) 
    const float densityRei = exp(atmosphere.rayleightDensity.layers[1].expScale * viewHeight);
    // ozone does not scatter light only absorbs 
    //const float densityOzo = atmosphere.getOzoneDensity(viewHeight); 

    // get exctincion coeficient of differnet particales in the atmosphere and scaled it based on the height 
    float3 mieScattering = atmosphere.mieScattering * densityMie;
    float3 rayleighScattering = atmosphere.rayLeightScattering * densityRei;
    float3 ozoneScattering = float3(0.0f); // does not scatter light only absorbs

    mieScatter = mieScattering;
    rayScatter = rayleighScattering;

    return mieScattering + rayleighScattering + ozoneScattering;
}

/*
 Computes the lenght of integration based on the ray-sphere intersection tests (sphere being the plane)
*/
public float ComputeIntegrationLength(AtmosphereParamsGPU atmosphere, float3 worldPosition, float3 worldDirection){
    
    float3 planet0 = float3(0.0);
    float planetIntersectionDistance = raySphereIntersectNearest(
        worldPosition, worldDirection, planet0, atmosphere.radiusBottom);
    float atmosphereIntersectionDistance = raySphereIntersectNearest(
        worldPosition, worldDirection, planet0, atmosphere.radiusTop);

    float integrationLength;
    if((planetIntersectionDistance == -1.0 && atmosphereIntersectionDistance > 0.0)){
        /* Ray missies both atmosphere and ground so we are not gonna ray march*/
        return -1.0;
    }    
     else if((planetIntersectionDistance == -1.0) && (atmosphereIntersectionDistance > 0.0)){
        /* ray intersects only atmosphere */
        integrationLength = atmosphereIntersectionDistance;
    }
    else if((planetIntersectionDistance > 0.0) && (atmosphereIntersectionDistance == -1.0)){
        /* ray intersects only planet */
        integrationLength = planetIntersectionDistance;
    } else {
        /* ray intersects both planet and atmosphere -> return the first intersection */
        integrationLength = min(planetIntersectionDistance, atmosphereIntersectionDistance);
    }

    return integrationLength;
}


//==================================================================================================
//===================== TRANSMITANCE PRECOMPUTATION ================================================
//==================================================================================================

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
public float2 GetTransmittanceTextureUvFromRMu(AtmosphereParamsGPU atmosphere, float r, float mu){
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

