// very simple compute shader to upscale the image to the target resolution 

Sampler2D _inputImage;
RWTexture2D _outputImage;

[[numthreads(16, 16, 1)]]
void computeMain(uint3 threadID : SV_DispatchThreadID){
    float2 dims;
    _outputImage.GetDimensions(dims.x, dims.y);

    float2 texCoords = (float2(threadID.xy) + 0.5 ) / dims;
    
    _outputImage[threadID.xy] = _inputImage.Sample(texCoords);
}