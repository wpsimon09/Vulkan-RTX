module intersections;
import Utils;

public float2 intersectRaySphere(float3 c, float r, float3 o, float3 d) {
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

/**
 * Return distance of the first intersection between ray and sphere
 * @param r0 - ray origin
 * @param rd - normalized ray direction
 * @param s0 - sphere center
 * @param sR - sphere radius
 * @return return distance of intersection or -1.0 if there is no intersection
 */
public float raySphereIntersectNearest(float3 r0, float3 rd, float3 s0, float sR)
{
	float a = dot(rd, rd);
	float3 s0_r0 = r0 - s0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sR * sR);
	float delta = b * b - 4.0*a*c;
	if (delta < 0.0 || a == 0.0)
	{
		return -1.0;
	}
	float sol0 = (-b - SafeSqrt(delta)) / (2.0*a);
	float sol1 = (-b + SafeSqrt(delta)) / (2.0*a);
	if (sol0 < 0.0 && sol1 < 0.0)
	{
		return -1.0;
	}
	if (sol0 < 0.0)
	{
		return max(0.0, sol1);
	}
	else if (sol1 < 0.0)
	{
		return max(0.0, sol0);
	}
	return max(0.0, min(sol0, sol1));
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
    if(delta < 0){
        t = -1;
        return false;
    }
    t = (-B + (C <= 0 ? sqrt(delta) : -sqrt(delta))) / (2 * A);
    return (C <= 0) | (B <= 0);
}

bool intersectRaySphere(float3 o, float3 d, float R, out float t){
    float A = dot(d, d);
    float B = 2 * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4 * A * C;
    if(delta < 0){
        t = -1.0;
        return false;
    }

    t = (-B + (C <= 0 ? sqrt(delta) : -sqrt(delta))) / (2 * A);
    return (C <= 0) | (B <= 0);
}