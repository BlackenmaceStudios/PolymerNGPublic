#include "guishader.hlsli"

struct VertexShaderInput
{
	float4 position  : POSITION;
	float2 texcoord0 : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input) 
{
	VertexShaderOutput output;

	float4x4 ui_matrix	 = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 1024.0f / 768.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0001f, -0.0001f,
									0.0f, 0.0f, 1.0f, 0.0f);

	float4 vertex = mul(ui_matrix, float4(input.position.xyz, 1.0));
	output.position = vertex;
	output.texcoord0 = input.texcoord0;

	return output;
}