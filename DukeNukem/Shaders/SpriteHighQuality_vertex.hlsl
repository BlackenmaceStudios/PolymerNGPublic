#include "AlbedoSimple.hlsli"

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix mWorldViewProjection;
	matrix mWorldView;
};


struct VertexShaderInput
{
	float4 position  : POSITION;
	float2 texcoord0 : TEXCOORD0;
};

float3 TransformPoint(float3 inpos)
{
	//	float3 pos;
	//	pos[0] = inpos.x * spriteTransformMatrix[0][0] + inpos.y * spriteTransformMatrix[1][0] + inpos.z * spriteTransformMatrix[2][0] + spriteTransformMatrix[3][0];
	//	pos[1] = inpos[0] * spriteTransformMatrix[0][1] + inpos[1] * spriteTransformMatrix[1][1] + inpos[2] * spriteTransformMatrix[2][1] + spriteTransformMatrix[3][1];
	//	pos[2] = inpos[0] * spriteTransformMatrix[0][2] + inpos[1] * spriteTransformMatrix[1][2] + inpos[2] * spriteTransformMatrix[2][2] + spriteTransformMatrix[3][2];
	//	return pos;
	return inpos;
}

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 vertex = mul(mWorldViewProjection, float4(TransformPoint(input.position.xyz), 1.0));
	output.position = vertex;
	output.position.w += 0.001;
	output.texcoord0 = input.texcoord0;
	output.texcoord1 = mul(mWorldView, float4(input.position.xyz, 1.0));
	return output;
}