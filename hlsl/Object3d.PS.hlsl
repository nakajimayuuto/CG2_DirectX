#include "Object3d.hlsli"

struct Material{
    float32_t4 color;
    int32_t lightingType;
    float32_t4x4 uvTransform;
    float32_t shininess;
    int32_t reflectionType;
};

struct DirectionalLight{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

struct PointLight{
    float32_t4 color;
    float32_t3 position;
    float intensity;
    float radius;
    float decay;
};

struct Camera{
    float32_t3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLight> gPointLight : register(b3);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

float32_t3 GetDiffuse(
float32_t3 normal,
float32_t3 textureColorRGB,
float32_t3 lightColorRGB,
float32_t3 lightDirection,
float lightIntensity
){
    float32_t3 diffuse = {0.0f,0.0f,0.0f};
    
    if (gMaterial.lightingType == 1){
    // Half Lambert
        float NdotL = dot(normalize(normal), -lightDirection);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    
        diffuse = gMaterial.color.rgb * textureColorRGB * (lightColorRGB * cos * lightIntensity);
    }else if (gMaterial.lightingType == 2){
    // Lambert Model
        float cos = saturate(dot(normalize(normal), -lightDirection));
    
        diffuse = gMaterial.color.rgb * textureColorRGB * (lightColorRGB * cos * lightIntensity);
    }
    
    return diffuse;
}

float32_t3 GetSpecular(
float32_t3 worldPosition,
float32_t3 normal,
float32_t3 textureColorRGB,
float32_t3 lightColorRGB,
float32_t3 lightDirection,
float lightIntensity
){
    float32_t3 specular = { 0.0f, 0.0f, 0.0f };
    
    float32_t3 toEye = normalize(gCamera.worldPosition - worldPosition);
    float32_t3 reflectLight = reflect(lightDirection, normalize(normal));
    
    float specularPow;
    
    if (gMaterial.reflectionType == 1) {
    // Phong Reflection
        float RdotE = dot(reflectLight, toEye);
        specularPow = pow(saturate(RdotE), gMaterial.shininess);
        specular = lightColorRGB * lightIntensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
    }else if (gMaterial.reflectionType == 2) {
    // Blinn Phong Reflection
        float32_t3 halfVector = normalize(-lightDirection + toEye);
        float NDotH = dot(normalize(normal), halfVector);
        specularPow = pow(saturate(NDotH), gMaterial.shininess);
        specular = lightColorRGB * lightIntensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
    }
    
    return specular;
}

float32_t3 GetOutputRGB(
float32_t3 worldPosition,
float32_t3 normal,
float32_t3 textureColorRGB,
float32_t3 lightColorRGB,
float32_t3 lightDirection,
float lightIntensity
){
    float32_t3 outputColorRGB = { 0.0f,0.0f,0.0f};
    
    outputColorRGB = outputColorRGB + GetDiffuse(normal,textureColorRGB,lightColorRGB,lightDirection,lightIntensity);
    outputColorRGB = outputColorRGB + GetSpecular(worldPosition,normal, textureColorRGB, lightColorRGB, lightDirection, lightIntensity);

    return outputColorRGB;
}

PixelShaderOutput main(VertexShaderOutput input){
    PixelShaderOutput output;
    
    float4 transformedUV = mul(float32_t4(input.texcoord,0.0f,1.0f),gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler,transformedUV.xy);
    
    if (textureColor.a <= 0.5f){
        discard;
    }
    
    if (gMaterial.lightingType == 0){
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
    }else{
        output.color.rgb = float32_t3(0.0f, 0.0f, 0.0f);
    }
    
    float32_t3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
    float32_t3 distance = length(gPointLight.position - input.worldPosition);
    float32_t3 factor = pow(saturate(-distance / gPointLight.radius + 1.0f),gPointLight.decay);
    
    output.color.rgb = output.color.rgb + GetOutputRGB(input.worldPosition, input.normal, textureColor.rgb, gDirectionalLight.color.rgb, gDirectionalLight.direction, gDirectionalLight.intensity);
    output.color.rgb = output.color.rgb + GetOutputRGB(input.worldPosition, input.normal, textureColor.rgb, gPointLight.color.rgb, pointLightDirection, gPointLight.intensity) * factor;
    
    
    output.color.a = gMaterial.color.a * textureColor.a;
    
    
    if (output.color.a == 0.0f){
        discard;
    }
        
    return output;
}