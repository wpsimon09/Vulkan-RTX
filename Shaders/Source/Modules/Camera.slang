internal interface ICamera{
    internal RayDesc GenerateRayToScene(float2 pixelPos, inout uint u);
}

public struct ProjectiveCamera : ICamera{
    float4x4 worldToCamera; // view matrix 
    float4x4 cameraToWorld; // invViewMatrix
    float tanFloat2;
    float aspect; 

    float4 film = {0.0f};

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
        return CameraToWorld(rayCameraSpaca);
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

    float aparatureSize;
    float focalLength;

    __init (float aparatureSize, float focalLenght, float tanFloat2, float aspect, float4x4 WorldToCamera, float4x4 CameraToWorld){
        this.projectiveCamera = ProjectiveCamera(tanFloat2, aspect, WorldToCamera, CameraToWorld);
        this.aparatureSize = aparatureSize;
        this.focalLength = focalLenght;
    };


    public RayDesc GenerateRayToScene(float2 pixelPos, inout uint u){
        var ray = projectiveCamera.GenerateRayToScene(pixelPos, u);

        return RayDesc();
    }

}