import RandomValues;

public struct ThinCameraParameters{
    public float aparatureSize = {0.0f};
    public float focalLength = {0.0f};
    public float imagePlaneDistance = {0.0f};
}

internal interface ICamera{
    /*
    Generates ray in the camera space, it is 
    */
    internal RayDesc GenerateRayToScene(float2 pixelPos, inout uint u);
}

public struct ProjectiveCamera : ICamera{
    float4x4 worldToCamera; // view matrix 
    float4x4 cameraToWorld; // invViewMatrix
    float tanFloat2;
    float aspect; 

    public float3 film = {0.0f};

    public __init (float tanFloat2, float aspectRatio, float4x4 WorldToCamera, float4x4 CameraToWorld){
        worldToCamera = WorldToCamera;
        cameraToWorld = CameraToWorld;
        this.tanFloat2 = tanFloat2;
        aspect = aspectRatio;
    };

    // generates ray in world space
    public RayDesc GenerateRayToScene(float2 pixelPos, inout uint u){
        // first generate ray in camera space 

        float2 pixel = float2(pixelPos.x, -pixelPos.y);

        float tanHalfFovX  = tanFloat2 * aspect;
        float tanHalfFovY  = tanFloat2;

        float3 direction = normalize(float3(
            pixel.x * tanHalfFovX,
            pixel.y * tanHalfFovY,
            -1.0f));  


        let rayCameraSpaca = RayDesc(float3(0.0f), 0.0f, direction, 10000.0f);        

        // convert the generated ray to world space where light transport
        return rayCameraSpaca;
    }

    RayDesc CameraToWorld(RayDesc inRay){
        RayDesc result;
        result.TMax = inRay.TMax;
        result.TMin = inRay.TMin;
        result.Direction = mul(float3x3(cameraToWorld), inRay.Direction);
        // third row in inverse view is camera position 
        result.Origin = float3(cameraToWorld._m03, cameraToWorld._m13, cameraToWorld._m23);
        return result;
    }

    RayDesc WorldToCamera(){

    }
}

public struct ThinLensCamera : ICamera{
    internal ProjectiveCamera projectiveCamera;

    ThinCameraParameters p;

    public float3 film = {0.0f};

    public __init (ThinCameraParameters params,  float tanFloat2, float aspect, float4x4 WorldToCamera, float4x4 CameraToWorld){
        this.projectiveCamera = ProjectiveCamera(tanFloat2, aspect, WorldToCamera, CameraToWorld);
        p = params;
    };


    public RayDesc GenerateRayToScene(float2 pixelPos, inout uint u){
        var ray = projectiveCamera.GenerateRayToScene(pixelPos, u);

        //ration of focal lenth to aparature size
        float fStop = p.focalLength / p.aparatureSize;

        float2 lensOffset = float2(RandomValue(u), RandomValue(u));
        
        float theta = lensOffset.x * 2.0f * float.getPi();
        float radius = lensOffset.y;

        float u = cos(theta) * sqrt(radius);
        float v = sin(theta) * sqrt(radius);

        float focusPlane = (p.imagePlaneDistance * p.focalLength) / (p.imagePlaneDistance - p.focalLength);

        float3 focusPoint = ray.Direction * (focusPlane / dot(ray.Direction, float3( 0.0f, 0.0f, -1.0f)));

        float circleOfConfusionRadius = p.focalLength / (2.0f * fStop);

        float3 origin = float3 (1.f, 0.f, 0.f) * (u * circleOfConfusionRadius ) + float3
                        (0.f, 1.f, 0.f) * (v * circleOfConfusionRadius);
        
        float3 direction = normalize(focusPoint - origin);

        return RayDesc(origin, ray.TMin, direction, ray.TMax);
    }

    public RayDesc CameraToWorld(RayDesc inRay){
        return projectiveCamera.CameraToWorld(inRay);
    }

}