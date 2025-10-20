internal interface ICamera{
    internal RayDesc GenerateRayToScene(float2 pixelPos, inout uint u);
}

public struct ProjectiveCamera : ICamera{
    float4x4 worldToCamera; // view matrix 
    float4x4 cameraToWorld; // invViewMatrix
    float4x4 projection;
    float2 dims;
    float FOV;

    float4 film = {0.0f};

    public __init (float fov, float2 dims, float4x4 projection, float4x4 WorldToCamera, float4x4 CameraToWorld){
        worldToCamera = WorldToCamera;
        cameraToWorld = CameraToWorld;
        projection = projection;
        FOV = fov;
        this.dims = dims;
    };

    // generates ray in world space
    public RayDesc GenerateRayToScene(float2 pixelPos, inout uint u){
        // first generate ray in camera space 
        float aspect = dims.x / dims.y;
        float tanHalfAngle = tan(FOV * 0.5f);
        float3 direction = normalize(float3(
            pixelPos.x * tanHalfAngle,
            pixelPos.y * tanHalfAngle / aspect,
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
        result.Origin = transpose(cameraToWorld)[3].xyz;
        return result;
    }

    RayDesc WorldToCamera(){

    }
}

public struct ThinLensCamera : ICamera{
    internal ProjectiveCamera projectiveCamera;

    float aparatureSize;
    float focalLength;

    __init (float aparatureSize, float focalLenght,float fov, float2 dims ,float4x4 projection, float4x4 WorldToCamera, float4x4 CameraToWorld){
        this.projectiveCamera = ProjectiveCamera(fov, dims, projection, WorldToCamera, CameraToWorld);
        this.aparatureSize = aparatureSize;
        this.focalLength = focalLenght;
    };


    public RayDesc GenerateRayToScene(float2 pixelPos, inout uint u){
        var ray = projectiveCamera.GenerateRayToScene(pixelPos, u);

        return RayDesc();
    }

}