import Material;

// B - brdf equation
interface ILightEnv {
    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo);
}

struct DirectionalLight : ILightEnv {
    float4 direction;
    float4 intensity;

    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        return (intensity.xyz * intensity.w).xyz * brdf.Evaluate(wo, direction.xyz, g.normal);
    }
}

struct PointLight : ILightEnv {
    float4 intensity;
    float4 position;
    float4 CLQU_Parameters; //constant, linear ,quadratic, inuse

    float3 illuminate<B:IBRDF>(SurfaceGeometry  g, B brdf, float3 wo){
        float3 delta = position.xyz - g.position;
        float distance = length(delta);
        float3 direction = normalize(delta);

        
        float  attentuation = 1.0f / (CLQU_Parameters.x + (CLQU_Parameters.y * distance) + (CLQU_Parameters.z * (distance * distance)));
        float3 illuminance = (intensity.xyz * intensity.w).xyz;
        return position.w == 1 ? illuminance * attentuation * brdf.Evaluate(wo, direction, g.normal) : 0.f;
    }
}

struct AreaLight : ILightEnv {
    float4 intensity;
    bool isInuse;
    bool TwoSided;
    float2 padding;
    float4 points[4];

    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        if (isInuse) {
            return (intensity.xyz * intensity.w).xyz * brdf.EvaluateByLTC(normalize(wo), g.position, g.normal, points, TwoSided);
        } else {
            return 0.f;
        }
     
    }
}
// N - number of lights 
// L - light type
struct LightArray<L : ILightEnv, let N : int> : ILightEnv {
    L lights[N];
    

    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        float3 sum = 0;
        for (int ii = 0; ii < N; ++ii) {
            sum += lights[ii].illuminate(g, brdf, wo);
        }
        return sum;
    }
}

// Empty light enviroment
struct EmptyLightEnv : ILightEnv {
    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        return 0;
    }
}

struct AmbientLight : ILightEnv {
    float aoIntensity;
    float3 albedo;
    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        return albedo * aoIntensity;
    }
}

// light pari allows us to create any light enviroment 
// we can create scene of lights for 3 light types like this 
// LightPair<LightPair<directional light, point light>, arealight>
struct LightPair<T : ILightEnv, U : ILightEnv> : ILightEnv {
    T first;
    U second;

    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        return first.illuminate(g, brdf, wo) + second.illuminate(g, brdf, wo);
    }
}

