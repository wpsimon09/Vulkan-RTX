//
// Created by wpsimon09 on 19/01/25.
//
#include "IntersectionTests.hpp"

#include "Application/Logger/Logger.hpp"

bool ApplicationCore::AABBRayIntersection(Ray& r, VulkanStructs::Bounds* b)
{
    float tmin = 0.001;
    float tmax = r.length;

    for (int a = 0; a < 3; a++) {
        float invD = 1.0f / r.direction[a];
        float t0 = (b->min[a] - r.origin[a]) * invD;
        float t1 = (b->max[a] - r.origin[a]) * invD;

        if (invD < 0.0f)
            std::swap(t0, t1);

        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;

        if (tmax <= tmin)
        {
            Utils::Logger::LogSuccess("Miss");
            return false;
        }
    }
    Utils::Logger::LogSuccess("Intersection");
    return true;
}
