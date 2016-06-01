#include "AlbedoSimple.hlsli"

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4 shadeOffsetVisibility;
	float4 fogDensistyScaleEnd;
	float4 fogColor;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

Texture1D paletteTexture : register(t1);
SamplerState paletteTextureSampler : register(s1);

Texture2D lookupTexture : register(t2);
SamplerState lookupTextureSampler : register(s2);

float CalculateFogFactor(float4 position)
{
	float fragDepth = position.z / position.w; 
	float fogFactor = 0;
	float fogDensity = fogDensistyScaleEnd.x;
	float fogScale = fogDensistyScaleEnd.y;
	float fogEnd = fogDensistyScaleEnd.z;

	// We are going to use linear fog in PolymerNG.
	fogFactor = 1.0 - ((0.85) - (fragDepth * 0.65217));
	fogFactor = clamp(fogFactor, 0.0, 1.0f);
	return fogFactor;
}

float4 main(VertexShaderOutput input) : SV_Target
{
	float shadeLookup = length(input.texcoord1.xyz) / 1.07 * shadeOffsetVisibility.y;
	shadeLookup = shadeLookup + shadeOffsetVisibility.x;

	float clamped_shade = min(max(shadeOffsetVisibility.x * 1.0, 0), 38);
	float modulation = ((38 - clamped_shade) / 38) * 1.69;

	float2 st = input.texcoord0.xy;
	float colorIndex = diffuseTexture.Sample(diffuseTextureSampler, st) * 256.0f;
	float colorIndexNear = lookupTexture.Sample(lookupTextureSampler, float2(colorIndex, floor(shadeLookup))).r;
	float colorIndexFar = lookupTexture.Sample(lookupTextureSampler, float2(colorIndex, floor(shadeLookup + 1.0))).r;
	float colorIndexFullbright = lookupTexture.Sample(lookupTextureSampler, float2(colorIndex, 0.0)).r;

	float alpha = 1.0f;
#ifndef ALBEDO_HQ_DIFFUSE	

	
	float3 texelNear = paletteTexture.Sample(paletteTextureSampler, colorIndexNear).rgb;
	float3 texelFar = paletteTexture.Sample(paletteTextureSampler, colorIndexFar).rgb;
	//diffuseTexel.rgb = paletteTexture.Sample(paletteTextureSampler, colorIndexFullbright).rgb;
	if (colorIndex == 256.0f)
	{
		discard;
	}
#else
	float3 modifiedColor = paletteTexture.Sample(paletteTextureSampler, colorIndexNear).rgb;
	float3 normalColor = paletteTexture.Sample(paletteTextureSampler, colorIndex * (1.0f/256.0f)).rgb;


	float4 colorBuffer = diffuseTexture.Sample(diffuseTextureSampler, st);

	colorBuffer.xyz = colorBuffer.xyz;
	alpha = colorBuffer.a;

	float3 texelNear, texelFar;
	texelNear.xyz = texelFar.xyz = colorBuffer;
#endif
	
	float3 result = lerp(texelNear, texelFar, frac(shadeLookup)) * float3(modulation, modulation, modulation);
	result = lerp(fogColor, result, CalculateFogFactor(input.position));
	return float4(result.x, result.y, result.z, alpha);

}