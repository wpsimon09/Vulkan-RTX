module Utils;

public float inverseSqrt(float x) {
    return max(1.0 / sqrt(x), 0.f);
}

public float4x4 inverse(float4x4 m) {
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
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

public float3 tonemap_aces(const float3 rgb) {
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

public RayDesc InitRay(float3 origin, float3 direction, float tMax)
{
    RayDesc rayDescription;
    rayDescription.Origin = origin;
    rayDescription.Direction = direction;
    rayDescription.TMax = tMax;
    rayDescription.TMin = 0.01;

    return rayDescription;
}

public bool IntersectsP(RayDesc ray, RaytracingAccelerationStructure as){
    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;
    rayQuery.TraceRayInline(as, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF,ray );

    rayQuery.Proceed();

    return (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT);
     
}

public float3x3 angleAxis3x3(float angle, float3 axis){
    float c, s;
    sincos(angle, s, c); // returns cosine and sine of the angle to the s and c

    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return float3x3(
        t * x * x + c,      t * x * y - s * z,  t * x * z + s * y,
        t * x * y + s * z,  t * y * y + c,      t * y * z - s * x,
        t * x * z - s * y,  t * y * z + s * x,  t * z * z + c
    );
}

public float3 WorldPosFromDepth(float depth, float2 uv, float4x4 inverseProj, float4x4 inverseView) {
    float4 clipSpacePos = float4(uv * 2.0 - 1.0, depth, 1.0);
    float4 viewSpacePos = mul(inverseProj, clipSpacePos);
    viewSpacePos /= viewSpacePos.w;
    float4 worldSpacePos = mul(inverseView, viewSpacePos);
    return worldSpacePos.xyz;
}

public float3 WorldPosFromLinearDepth(float32_t depth,float2 uv, float4x4 inverseProj, float4x4 inverseView){
   float z = depth;
    float4 clipSpacePos = float4(uv * 2.0 - 1.0, 1.0, 1.0);
    float4 viewRay = mul( inverseProj, clipSpacePos);
    
    viewRay.xyz /= viewRay.w;

    float3 viewSpacePos = normalize(viewRay.xyz) * depth;

    float4 worldSpacePosition = mul(inverseView, float4(viewSpacePos,1.0));

    return worldSpacePosition.xyz;
}

public float LinearizeDepth(float d,float zNear,float zFar)
{
    return  zNear * zFar / (zFar - d * (zFar - zNear));
}

/*
* Returns angle of the calcualted cone 
* r - radius
* o - oriding
* d - direction
*/
public float CalculateConeAngle(float r, float3 o, float3 d){
    float3 lightPosition = o + 1000.0 * (-d);

    //=======================
    // Soft shadow calculation
    float3 Lperp = cross(d, float3(0,1,0)); //perpendicular to the world normal


    if(all(Lperp == 0.0f)) {Lperp.x = 1.0;}

    //================================================
    // get the vecor from light center to light edge
    float3 toLightEdge = normalize((lightPosition + Lperp * r) - o);

    //==================================================
    // Angle between L and toLightEdge used for sampling
    float coneAngle = acos(dot(d, toLightEdge)) * 2.0f ; // multiply by 2 since we only have half angle 

    return coneAngle;
}