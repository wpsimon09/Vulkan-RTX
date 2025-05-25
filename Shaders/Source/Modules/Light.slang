module Light;

import Material;
import Surface;

[[vk::binding(12, 0)]]
ConstantBuffer<SamplerCube> IrradianceMap;

[[vk::binding(13, 0)]]
ConstantBuffer<SamplerCube> PrefilterMap;

[[vk::binding(14, 0)]]
ConstantBuffer<Sampler2D> BRDF_Lut;

public struct LightInfo {
     public DirectionalLight dirLight;
     // PointLight[10] pointLigths;
     public LightArray<PointLight, 20> pointLights;
     public LightArray<AreaLight, 4> areaLights;
     public float4 info;
}

// B - brdf equation
public interface ILightEnv {
    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo);
}

public struct DirectionalLight : ILightEnv {
    public float4 direction;
    public float4 intensity;


    public float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        return (intensity.xyz * intensity.w).xyz * brdf.Evaluate(wo, direction.xyz, g.normal);
    }
}

public struct PointLight : ILightEnv {
    public float4 intensity;
    public float4 position;
    public float4 CLQU_Parameters; //constant, linear ,quadratic, inuse

    public float3 illuminate<B:IBRDF>(SurfaceGeometry  g, B brdf, float3 wo){
        float3 delta = position.xyz - g.position;
        float distance = length(delta);
        float3 direction = normalize(delta);

        
        float  attentuation = 1.0f / (CLQU_Parameters.x + (CLQU_Parameters.y * distance) + (CLQU_Parameters.z * (distance * distance)));
        float3 illuminance = (intensity.xyz * intensity.w).xyz;
        return position.w == 1 ? illuminance * attentuation * brdf.Evaluate(wo, direction, g.normal) : 0.f;
    }
}

public struct AreaLight : ILightEnv {
    public float4 intensity;
    public bool isInuse;
    public bool TwoSided;
    public float2 padding;
    public float4 points[4];

    public float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        if (isInuse) {
            return (intensity.xyz * intensity.w).xyz * brdf.EvaluateByLTC(normalize(wo), g.position, g.normal, points, TwoSided);
            //return 0.f;
        } else {
            return 0.f;
        }
     
    }
}
// N - number of lights 
// L - light type
public struct LightArray<L : ILightEnv, let N : int> : ILightEnv {
    public L lights[N];
    

    public float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        float3 sum = 0;
        for (int ii = 0; ii < N; ++ii) {
            sum += lights[ii].illuminate(g, brdf, wo);
        }
        return sum;
    }
}

// Empty light enviroment
public struct EmptyLightEnv : ILightEnv {
    public float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        return 0;
    }
}

public struct AmbientLight : ILightEnv {
    public float aoIntensity;
    public float3 albedo;
    public bool applyIBL;
    public float roughness;
    
    public float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        if (applyIBL) { // passed from application based on if the env is nullptr or not 
            float3 kS = brdf.FresnellShlickRoughness(max(dot(g.normal, wo), 0.0));
            float3 kD = float3(1.0) - kS;
            float3 irradiance = IrradianceMap.Sample(g.normal.xyz).xyz;
            float3 diffuse = irradiance * albedo;

            float3 R = reflect(-wo, g.normal);
            const float MAX_REFLECTION_LOD = 9.0;

            float3 prefilteredColor = PrefilterMap.SampleLevel(R, roughness * MAX_REFLECTION_LOD).xyz;

            float2 envBrdf = BRDF_Lut.Sample(float2(max(dot(g.normal, wo), 0.0f), roughness)).xy;
            float3 spec = prefilteredColor * (kS * envBrdf.x + envBrdf.y);
            
            return (kD * diffuse + spec) * aoIntensity;
        } else {
            return albedo * 0.7;
        }
    }
}

// light pari allows us to create any light enviroment 
// we can create scene of lights for 3 light types like this 
// LightPair<LightPair<directional light, point light>, arealight>
public struct LightPair<T : ILightEnv, U : ILightEnv> : ILightEnv {
    public T first;
    public U second;

    public float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        return first.illuminate(g, brdf, wo) + second.illuminate(g, brdf, wo);
    }
}

