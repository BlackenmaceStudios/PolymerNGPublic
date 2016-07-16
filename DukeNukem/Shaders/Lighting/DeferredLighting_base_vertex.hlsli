#include "../guishader.hlsli"

struct VertexShaderInputWithoutBuffers
{
	uint vertexid : SV_VERTEXID;
};

VertexShaderOutput main(VertexShaderInputWithoutBuffers input)
{
	VertexShaderOutput output;

	float2 texcoord = float2(input.vertexid & 1, input.vertexid >> 1);
	float4 vertex = float4((texcoord.x - 0.5f) * 2, -(texcoord.y - 0.5f) * 2, 0, 1);
	output.position = vertex;
	output.texcoord0 = texcoord;

	return output;
}