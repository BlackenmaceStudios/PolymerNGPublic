#include "guishader.hlsli"
#include "Utility/ShaderUtility.hlsli"

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 st = input.texcoord0.xy;

	// Grab the color from the diffuse render target. If alpha == 0(sky) then discard.
	float4 color = diffuseTexture.Sample(diffuseTextureSampler, st);

	return color;
}