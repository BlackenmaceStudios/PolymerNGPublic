#include "guishader.hlsli"

struct VertexShaderInput
{
	float4 position  : POSITION;
	float2 texcoord0 : TEXCOORD0;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	float4x4 projection_matrix;
};


VertexShaderOutput main(VertexShaderInput input) 
{
	VertexShaderOutput output;

	float4 vertex = mul(projection_matrix, float4(input.position.xyz, 1.0));
	output.position = vertex;
	output.texcoord0 = input.texcoord0;

	return output;
}