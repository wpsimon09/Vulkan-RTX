module AtmosphereParameters;

public struct AtmosphereParamsCPU{
    float4 groundAlbedo; //xyz - colour, w: Radius bottom
    float4 rayleighScattering; //xyz - wavelenght dependad, w :radius top
    float4 mieScattering;  //xyz: as-is, w:Reayleigh density exp scale 
    float4 mieAbsorption;//xyz: as-is, w: Mie density exp scale
    float4 mieExctinction;//xyz: as-is, w: Mie phase G
    float4 absorptionExctinction; //xyz: as-is, w: Absorption denssity 0 layer width
    int4 booleans; //x - use multiscattering

    public AtmosphereParamsGPU Parse(){
        AtmosphereParamsGPU gpu;

        gpu.groundAlbedo = groundAlbedo.xyz;
        gpu.radiusBottom = groundAlbedo.w;

        gpu.radiusTop = rayleighScattering.w;
        
        gpu.reayleghtDensityExpScale = mieScattering.w;
        
        gpu.mieDensityExpScale = mieAbsorption.w;
        
        gpu.absorptionDensityZeroLayerWidth = absorptionExctinction.w;
        
        gpu.transmitanceTextureWidth  = 256.0f; 
        gpu.transmitanceTextureHeight = 64.0f;  
        
        gpu.rayleightDensity.layers[0] = DensityProfileLayer();
        gpu.rayleightDensity.layers[1] = DensityProfileLayer(0.0f, 1.0f, -1.0f / 8.0f);
        gpu.rayLeightScattering = rayleighScattering.xyz;
        
        gpu.mieDensity.layers[0] = DensityProfileLayer();
        gpu.mieDensity.layers[1] = DensityProfileLayer(0.0f, 1.0f, -1.0f / 1.2f);
        
        gpu.mieExtinction = mieExctinction.xyz;
        gpu.mieScattering = mieScattering.xyz;
        gpu.miePhaseG = mieExctinction.w;
        
        
        gpu.ozoneDensity.layers[0] = DensityProfileLayer(25.0f, 0.0f, 0.0f, 1.0f / 15.0f, -2.0f / 3.0f);
        gpu.ozoneDensity.layers[1] = DensityProfileLayer(0.0f, 0.0f, 0.0f, -1.0f / 15.0f, 8.0f / 3.0f);
        gpu.absorptionExctinction = absorptionExctinction.xyz;

        gpu.UseMultiscattering = (bool)booleans.x;
        gpu.DrawSun = (bool)booleans.y;

        return gpu;
    }
}


public struct AtmosphereParamsGPU{
    public float3 groundAlbedo;
    public float radiusBottom;

    public float3 rayLeightScattering;
    public float radiusTop;

    public float3 mieScattering;
    public float reayleghtDensityExpScale;
    
    public float mieDensityExpScale;
    public float3 mieExtinction;
    
    public float miePhaseG;
    
    public float3 absorptionExctinction;
    public float absorptionDensityZeroLayerWidth;

    public float transmitanceTextureWidth;
    public float transmitanceTextureHeight;

    public DensityProfile rayleightDensity;
    public DensityProfile mieDensity;
    public DensityProfile ozoneDensity;


    public bool UseMultiscattering;
    public bool DrawSun;

    public float getOzoneDensity(float viewHeight) {
        float densityOzo;
        if (viewHeight < ozoneDensity.layers[0].width) {
            densityOzo = ozoneDensity.layers[0].exponentialTerm * exp(ozoneDensity.layers[0].expScale * viewHeight)
                       + ozoneDensity.layers[0].linearTerm * viewHeight
                       + ozoneDensity.layers[0].constantTerm;
        } else {
            densityOzo = ozoneDensity.layers[1].exponentialTerm * exp(ozoneDensity.layers[1].expScale * viewHeight)
                       + ozoneDensity.layers[1].linearTerm * viewHeight
                       + ozoneDensity.layers[1].constantTerm;
        }
        return clamp(densityOzo, 0.0, 1.0);
    }


    
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
public struct DensityProfileLayer{
    public float width = 0.0;
    public float exponentialTerm = 0.0;
    public float expScale = 0.0;
    public float linearTerm = 0.0;
    public float constantTerm = 0.0;

    __init(float _width = 0.0, float _exponentialTerm = 0.0, float _expScale = 0.0f, float _linearTerm = 0.0f, float _constantTerm = 0.0f){
        width = _width;
        exponentialTerm = _exponentialTerm;
        expScale = _expScale;
        linearTerm = _linearTerm;
        constantTerm = _constantTerm;
    }
}



public struct DensityProfile{
    // one density profile layer 
    public DensityProfileLayer layers[2];
}

