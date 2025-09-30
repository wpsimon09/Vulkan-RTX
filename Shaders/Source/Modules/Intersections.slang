float2 intersectRaySphere(float3 c, float r, float3 o, float3 d) {
    float3 of = o - c;
    const float a = 1.0;
    float b = 2.0 * dot(of, d);
    float c = dot(of, of) - r * r;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant > 0) {
        float s = sqrt(discriminant);
        float dstToSphereNear = max(0.0, (-b - s) / (2.0 * a));
        float dstToSphereFar = (-b + s) / (2.0 * a);

        if (dstToSphereFar >= 0.0) {
            return float2(dstToSphereNear, dstToSphereFar - dstToSphereNear);
        }
    }
    return float2(0.0, 0.0);
}

bool existsRayCircleIntersection(float2 o, float2 d, float R){
    float A = dot(d, d);
    float B = 2 * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4 * A * C;
    return (delta >= 0) && ((C <= 0) | (B <= 0));
}

bool existsRaySphereIntersection(float3 o, float3 d, float R){
    float A = dot(d, d);
    float B = 2 * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4 * A * C;
    return (delta >= 0) && ((C <= 0) | (B <= 0));
}

bool intersectRayCircle(float2 o, float2 d, float R, out float t){
    float A = dot(d, d);
    float B = 2 * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4 * A * C;
    if(delta < 0)
        return false;
    t = (-B + (C <= 0 ? sqrt(delta) : -sqrt(delta))) / (2 * A);
    return (C <= 0) | (B <= 0);
}

bool intersectRaySphere(float3 o, float3 d, float R, out float t){
    float A = dot(d, d);
    float B = 2 * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4 * A * C;
    if(delta < 0)
        return false;
    t = (-B + (C <= 0 ? sqrt(delta) : -sqrt(delta))) / (2 * A);
    return (C <= 0) | (B <= 0);
}