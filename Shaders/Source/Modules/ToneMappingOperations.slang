module ToneMappingOperations;

float Tonemap_Uchimura(float x, float P, float a, float m, float l, float c, float b) {
    // Uchimura 2017, "HDR theory and practice"
    // Math: https://www.desmos.com/calculator/gslcdxvipg
    // Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
    float l0 = ((P - m) * l) / a;
    float L0 = m - m / a;
    float L1 = m + (1.0 - m) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    float w0 = 1.0 - smoothstep(0.0, m, x);
    float w2 = step(m + l0, x);
    float w1 = 1.0 - w0 - w2;

    float T = m * pow(x / m, c) + b;
    float S = P - (P - S1) * exp(CP * (x - S0));
    float L = m + a * (x - m);

    return T * w0 + L * w1 + S * w2;
}


public float Tonemap_Uchimura(float x) {
    const float P = 1.0;  // max display brightness
    const float a = 1.0;  // contrast
    const float m = 0.22; // linear section start
    const float l = 0.4;  // linear section length
    const float c = 1.33; // black
    const float b = 0.0;  // pedestal
    return Tonemap_Uchimura(x, P, a, m, l, c, b);
}


// TONE MAPPING FUNCTIONS BY 
// https://64.github.io/tonemapping/
public float3 tone_map_aces_approx(float3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0f, 1.0f);
}


float3 uncharted2_tonemap_partial(float3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

public float3 Aces_Full(const float3 rgb) {
    // sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
    const float3x3 IN = float3x3(
      0.59719, 0.07600, 0.02840,
      0.35458, 0.90834, 0.13383,
      0.04823, 0.01566, 0.83777);

    // ODT_SAT => XYZ => D60_2_D65 => sRGB
    const float3x3 OUT = float3x3(
      1.60475, -0.10208, -0.00327,
      -0.53108,  1.10813, -0.07276,
      -0.07367, -0.00605,  1.07602);

    float3 col = mul( rgb, IN);

    const float3 a = col * (col + 0.0245786f) - 0.000090537f;
    const float3 b = col * (0.983729f * col + 0.4329510f) + 0.238081f;
    col = a / b;

    return clamp(mul( col, OUT), 0., 1.);
}

public float3 uncharted2_filmic(float3 v, float exposure)
{
    float exposure_bias = exposure + 1.0;
    float3 curr = uncharted2_tonemap_partial(v * exposure_bias);

    float3 W = float3(11.2f);
    float3 white_scale = float3(1.0f) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}

public float aces_film(float x)
{
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}
