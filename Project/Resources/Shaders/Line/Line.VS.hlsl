#include "Line.hlsli"

struct TransformationMatrix{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4x4 WorldInverseTranspose;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput{
    float32_t4 position : POSITION0;
    float32_t4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input){
    VertexShaderOutput output;
    output.position = mul(input.position,gTransformationMatrix.WVP);
    output.color = input.color;
    output.worldPosition = mul(input.position,gTransformationMatrix.World).xyz;
	return output;
}