#include "guishader.hlsli"


Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

Texture1D paletteTexture : register(t1);
SamplerState paletteTextureSampler : register(s1);

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 st = input.texcoord0.xy;
	float r = diffuseTexture.Sample(diffuseTextureSampler,st);
	if (r == 1)
	{
		discard;
	}
	float3 result = paletteTexture.Sample(paletteTextureSampler, r).xyz;
	return float4(result.x,result.y,result.z,1);
}