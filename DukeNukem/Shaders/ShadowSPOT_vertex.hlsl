#include "ShadowShared.hlsli"
#include "VertexShaderShared.hlsli"

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix mWorldViewProjection;
	float4 light_position_and_range;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4 vertex = mul(mWorldViewProjection, float4(input.position.xyz, 1.0));
	output.Position = vertex;
	output.texcoord0 = input.position * (1.0f / 1000.0f);
	output.diffuseTexCoords = input.texcoord0;
	output.lightpos_and_range = light_position_and_range * (1.0f / 1000.0f);
	output.lightpos_and_range.w = light_position_and_range.w;
	return output;
}
