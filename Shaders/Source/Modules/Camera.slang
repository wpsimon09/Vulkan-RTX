internal interface ICamera{
    internal RayDesc GenerateRayToScene(float2 pixelPos, inout uint u);
}

public struct ProjectiveCamera : ICamera{
    float4x4 worldToCamera; // view matrix 
    float4x4 cameraToWorld; // invViewMatrix
    float4x4 projection;

    float4 film = {0.0f};

    public __init (float4x4 projection, float4x4 WorldToCamera, float4x4 CameraToWorld){
        worldToCamera = WorldToCamera;
        cameraToWorld = CameraToWorld;
        projection = projection;
    };

    public RayDesc GenerateRayToScene(float2 pixelPos, inout uint u){
        RayDesc ray;

        ray.Origin = cameraToWorld[3].xyz;  
        ray.TMin = 0.0f;
        ray.TMax = float.maxValue;

        float tanHalfFoVY = 1.0f / projection[1][1];
        float aspect = projection[1][1] / projection[0][0];
        float tanHalfFoVX = tanHalfFoVY * aspect;

        pixelPos.y = -pixelPos.y;

        ray.Direction = normalize(
            pixelPos.x * tanHalfFoVX * cameraToWorld[0].xyz + 
            pixelPos.y * tanHalfFoVY * cameraToWorld[1].xyz + 
            cameraToWorld[2].xyz                              
        );

        return ray;
    }
}

public struct ThinLensCamera : ICamera{
    internal ProjectiveCamera projectiveCamera;

    float aparatureSize;
    float focalLength;

    __init (float aparatureSize, float focalLenght,float4x4 projection, float4x4 WorldToCamera, float4x4 CameraToWorld){
        this.projectiveCamera = ProjectiveCamera(projection, WorldToCamera, CameraToWorld);
        this.aparatureSize = aparatureSize;
        this.focalLength = focalLenght;
    };


    public RayDesc GenerateRayToScene(float2 pixelPos, inout uint u){
        var ray = projectiveCamera.GenerateRayToScene(pixelPos, u);

        return RayDesc();
    }

}