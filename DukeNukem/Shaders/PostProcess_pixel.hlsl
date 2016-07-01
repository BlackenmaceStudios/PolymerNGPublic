#include "guishader.hlsli"
#include "Utility/ShaderUtility.hlsli"

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4x4 invModelViewProjectionMatrix;
	float4x4 viewMatrix;
};

#define USE_SLOW_POSITION_METHOD

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

Texture2D lightTexture : register(t1);
SamplerState lightTextureSampler : register(s1);

Texture2D positionTexture : register(t2);
SamplerState positionTextureSampler : register(s2);

Texture2D depthTexture : register(t3);
SamplerState depthTextureSampler : register(s3);

#define USE_SLOW_POSITION_METHOD
float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 st = input.texcoord0.xy;

	// Grab the color from the diffuse render target. If alpha == 0(sky) then discard.
	float4 color = diffuseTexture.Sample(diffuseTextureSampler, st);
	if (color.a == 0)
		discard;

	float3 hdrLighting = lightTexture.Sample(lightTextureSampler, st).xyz;

	float weight[5];

	weight[0] = 0.227027;
	weight[1] = 0.1945946;
	weight[2] = 0.1216216;
	weight[3] = 0.054054;
	weight[4] = 0.016216;

	float width, height, levels;
	lightTexture.GetDimensions(0, width, height, levels);

	float2 tex_offset = 1.0 / float2(width, height); // gets size of single texel
	for (int i = 1; i < 5; ++i)
	{
		hdrLighting += lightTexture.Sample(lightTextureSampler, st + float2(tex_offset.x * i, 0.0)).rgb * weight[i];
		hdrLighting += lightTexture.Sample(lightTextureSampler, st - float2(tex_offset.x * i, 0.0)).rgb * weight[i];
	}

	for (int i = 1; i < 5; ++i)
	{
		hdrLighting += lightTexture.Sample(lightTextureSampler, st + float2(0.0, tex_offset.y * i)).rgb * weight[i];
		hdrLighting += lightTexture.Sample(lightTextureSampler, st + float2(0.0, tex_offset.y * i)).rgb * weight[i];
	}


	float3 finalColor = color.xyz * ((hdrLighting.xyz) + float3(0.1, 0.1, 0.1));

	return float4(finalColor.x, finalColor.y, finalColor.z, 1);
}