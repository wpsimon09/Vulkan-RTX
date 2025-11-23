module SBG_ShadowPass;
import Core;
import Light;

//=================================
// Set 0
[[vk::binding(0,0)]]
public ConstantBuffer<GlobalData> globalData;

// those two are for shadow calculations 
[[vk::binding(1, 0)]]
public ConstantBuffer<LightInfo> _lightInfo;

[[vk::binding(2, 0)]]
public RaytracingAccelerationStructure _tlas;

[[vk::binding(3, 0)]]
public Sampler2D _positionTexture;

[[vk::binding(4,0)]]
public Sampler2D _blueNoiseTextureArray;

[[vk::binding(5,0)]]
public Sampler2D _normalTexture;

public struct AoOcclusionParameters{
    public float radius;
    public float sampleCount;
    public float strenght;
}

[[vk::push_constant]]
public AoOcclusionParameters _aoParameters;


public static const float2 BlueNoiseInDisk[64] = float2[64](
    float2(0.478712,0.875764),
    float2(-0.337956,-0.793959),
    float2(-0.955259,-0.028164),
    float2(0.864527,0.325689),
    float2(0.209342,-0.395657),
    float2(-0.106779,0.672585),
    float2(0.156213,0.235113),
    float2(-0.413644,-0.082856),
    float2(-0.415667,0.323909),
    float2(0.141896,-0.939980),
    float2(0.954932,-0.182516),
    float2(-0.766184,0.410799),
    float2(-0.434912,-0.458845),
    float2(0.415242,-0.078724),
    float2(0.728335,-0.491777),
    float2(-0.058086,-0.066401),
    float2(0.202990,0.686837),
    float2(-0.808362,-0.556402),
    float2(0.507386,-0.640839),
    float2(-0.723494,-0.229240),
    float2(0.489740,0.317826),
    float2(-0.622663,0.765301),
    float2(-0.010640,0.929347),
    float2(0.663146,0.647618),
    float2(-0.096674,-0.413835),
    float2(0.525945,-0.321063),
    float2(-0.122533,0.366019),
    float2(0.195235,-0.687983),
    float2(-0.563203,0.098748),
    float2(0.418563,0.561335),
    float2(-0.378595,0.800367),
    float2(0.826922,0.001024),
    float2(-0.085372,-0.766651),
    float2(-0.921920,0.183673),
    float2(-0.590008,-0.721799),
    float2(0.167751,-0.164393),
    float2(0.032961,-0.562530),
    float2(0.632900,-0.107059),
    float2(-0.464080,0.569669),
    float2(-0.173676,-0.958758),
    float2(-0.242648,-0.234303),
    float2(-0.275362,0.157163),
    float2(0.382295,-0.795131),
    float2(0.562955,0.115562),
    float2(0.190586,0.470121),
    float2(0.770764,-0.297576),
    float2(0.237281,0.931050),
    float2(-0.666642,-0.455871),
    float2(-0.905649,-0.298379),
    float2(0.339520,0.157829),
    float2(0.701438,-0.704100),
    float2(-0.062758,0.160346),
    float2(-0.220674,0.957141),
    float2(0.642692,0.432706),
    float2(-0.773390,-0.015272),
    float2(-0.671467,0.246880),
    float2(0.158051,0.062859),
    float2(0.806009,0.527232),
    float2(-0.057620,-0.247071),
    float2(0.333436,-0.516710),
    float2(-0.550658,-0.315773),
    float2(-0.652078,0.589846),
    float2(0.008818,0.530556),
    float2(-0.210004,0.519896) 
);

public static const float goldenRatioConjugate = 0.61803398875f;
