#include "SkyBox.hlsli"

struct Material
{
    float32_t4 color;
    int32_t lightingType;
    float32_t4x4 uvTransform;
    float32_t shininess;
    int32_t reflectionType;
};

ConstantBuffer<Material> gMaterial : register(b0);

TextureCube<float32_t4> gTexture : register(t0);

SamplerState gSampler : register(s0);

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};


PixelShaderOutput main(VertexShaderOutput input){
    PixelShaderOutput output;
    
    float32_t4 textureColor = gTexture.Sample(gSampler,input.texcoord);
    output.color = textureColor * gMaterial.color;
    
    
    //if (output.color.a <= 0.0f){
    //    discard;
    //}
        
    return output;
}