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

    float3 illuminate<B:IBRDF>(SurfaceGeometry  g, B brdf, float3 wo){
        float3 delta = position.xyz - g.position;
        float distance = length(delta);
        float3 direction = normalize(delta);
        float3 illuminance = (intensity.xyz * intensity.w).xyz / (distance*distance);

        return illuminance * brdf.Evaluate(wo, direction, g.normal);
    }
}

// N - number of lights 
// L - light type
struct LightArray<L : ILightEnv, let N : int> : ILightEnv {
    int count;
    float padding [3];
    L lights[N];
    

    float3 illuminate<B : IBRDF>(SurfaceGeometry g, B brdf, float3 wo) {
        float3 sum = 0;
        for (int ii = 0; ii < count; ++ii) {
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

