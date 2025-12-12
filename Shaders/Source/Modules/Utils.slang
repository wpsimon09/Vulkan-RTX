module Utils;


public static const float m = 1.0;
public static const float m2 = m * m;


//===========================================

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


public RayDesc InitRay(float3 origin, float3 direction, float tMax)
{
    RayDesc rayDescription;
    rayDescription.Origin = origin;
    rayDescription.Direction = direction;
    rayDescription.TMax = tMax;
    rayDescription.TMin = 0.00001;

    return rayDescription;
}



public bool IntersectsP(RayDesc ray, RaytracingAccelerationStructure as, out float tHit){
    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;
    rayQuery.TraceRayInline(as, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);

    rayQuery.Proceed();

    if(rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT){
        tHit = rayQuery.CommittedRayT();
        return true;
    }
    tHit = ray.TMax;
    return false;
    
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

public double3 WorldPosFromDepth(double depth, float2 uv, float4x4 inverseProj, float4x4 inverseView) {
    double4 clipSpacePos = double4(uv * 2.0 - 1.0, depth, 1.0);
    double4 viewSpacePos = mul(inverseProj, clipSpacePos);
    viewSpacePos /= viewSpacePos.w;
    double4 worldSpacePos = mul(inverseView, viewSpacePos);
    return worldSpacePos.xyz;
}

public float3 WorldPosFromLinearDepth(float depth,float2 uv, float4x4 inverseProj, float4x4 inverseView){
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

public float LengthSquared(float3 vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
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


public float3x3 CalculateTBN(float3 geomNormal, float4 tangnent, float3x3 normalM){
    float3 T = normalize(mul(normalM, tangnent.xyz));
    float3 N = normalize(mul(normalM, geomNormal.xyz));
    float3 B = normalize(cross(N, T)) * tangnent.w;       

    float3x3 TBN = transpose(float3x3(T, B, N));
    return TBN;
}

public float3 PowFloat3(float3 v, float p){ 
    return float3(pow(v.x, p), pow(v.y, p), pow(v.z,p));
}

public float ClampCosine(float mu) {
  return clamp(mu, float(-1.0), float(1.0));
}

public float ClampDistance(float d) {
  return max(d, 0.0 * m);
}

public float SafeSqrt(float a) {
  return sqrt(max(a, 0.0 * (m2)));
}

public float4 getRotationToZAxis(float3 input){
    if (input.z < -0.99999f)
        return float4(1.0f, 0.0f, 0.0f, 0.0f);

    return normalize(float4(input.y, -input.x, 0.0f, 1.0f + input.z));
}

// Optimized point rotation using quaternion
// Source:
// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
public float3 rotatePoint(float4 q, float3 v) {
  const float3 qAxis = float3(q.x, q.y, q.z);
  return 2.0f * dot(qAxis, v) * qAxis + (q.w * q.w - dot(qAxis, qAxis)) * v +
         2.0f * q.w * cross(qAxis, v);
}


public float4 invertRotation(float4 q) { return float4(-q.x, -q.y, -q.z, q.w); }

public bool SameHemisphere(float3 w, float3 wp){
    return w.z* wp.z > 0;
}

public bool IsNan(float3 vec){
    return (isnan(vec.x) || isnan(vec.y) || isnan(vec.z));
}




