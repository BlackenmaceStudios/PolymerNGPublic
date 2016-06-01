#include "guishader.hlsli"

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4 modulationColor;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

Texture1D paletteTexture : register(t1);
SamplerState paletteTextureSampler : register(s1);

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 st = input.texcoord0.xy;
#ifdef GUISHADER_HIGHQUALITY
	float4 result = diffuseTexture.Sample(paletteTextureSampler, st);
	return result;
#else
	float r = diffuseTexture.Sample(diffuseTextureSampler,st);
	if (r == 1)
	{
		discard;
	}
	float3 result = paletteTexture.Sample(paletteTextureSampler, r).xyz * modulationColor;
#endif
	return float4(result.x,result.y,result.z,1);
}