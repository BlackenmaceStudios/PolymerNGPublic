#include "AlbedoSimple.hlsli"
#include "VertexShaderShared.hlsli"

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix mWorldViewProjection;
	matrix mWorldView;
};


VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 vertex = mul(mWorldViewProjection, float4(input.position.xyz, 1.0));
	output.position = vertex;
	output.texcoord0 = input.texcoord0;
	output.texcoord1 = mul(mWorldView, float4(input.position.xyz, 1.0));
	return output;
}