#include "AlbedoSimple.hlsli"
#include "Utility/ShaderUtility.hlsli"

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4 shadeOffsetVisibility;
	float4 fogDensistyScaleEnd;
	float4 fogColor;
	float4  normal;
	float4  tangent;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

Texture1D paletteTexture : register(t1);
SamplerState paletteTextureSampler : register(s1);

Texture2D lookupTexture : register(t2);
SamplerState lookupTextureSampler : register(s2);

#ifdef ALBEDO_USE_NORMALMAP
Texture2D normalMapTexture : register(t3);
SamplerState normalMapSampler : register(s3);

Texture2D specMapTexture : register(t4);
SamplerState specMapSampler : register(s4);
#endif

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

struct AlbedoPixelOut
{
	float depth : SV_DEPTH;
	float4 diffuseColor			: SV_Target0;
	float4 surfaceNormal		: SV_Target1;
	float4 normalMap			: SV_Target2;
	float4 tangent				: SV_Target3;
	float4 specularGlowProperty : SV_Target4;
};

AlbedoPixelOut main(VertexShaderOutput input)
{
	AlbedoPixelOut albedoPixelOut;

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

	float3 result = lerp(texelNear, texelFar, frac(shadeLookup)) * float3(modulation, modulation, modulation);
	result = lerp(fogColor, result, CalculateFogFactor(input.position));
#else
	float3 modifiedColor = float3(1, 1, 1); // paletteTexture.Sample(paletteTextureSampler, colorIndexNear).rgb;
	float3 normalColor = float3(1, 1, 1); // paletteTexture.Sample(paletteTextureSampler, colorIndex * (1.0f / 256.0f)).rgb;

	float4 colorBuffer = diffuseTexture.Sample(diffuseTextureSampler, st);

	colorBuffer.xyz = colorBuffer.xyz;
	alpha = colorBuffer.a;
	if (alpha == 0)
		discard;

	float3 texelNear, texelFar;
	texelNear.xyz = texelFar.xyz = colorBuffer;

	float3 result = colorBuffer.xyz;
#endif
	
	albedoPixelOut.diffuseColor = float4(result.x, result.y, result.z, alpha);
#ifdef SPRITE
	albedoPixelOut.specularGlowProperty.b = 1.0f;
#else
	albedoPixelOut.surfaceNormal.xyz = normal;
#endif
	albedoPixelOut.surfaceNormal.a = alpha; // input.position.z / input.position.w;
	albedoPixelOut.depth = input.vDepthVS.z / input.vDepthVS.w;
	
#ifdef ALBEDO_USE_NORMALMAP
	float4 normalMap = normalMapTexture.Sample(normalMapSampler, st);
	albedoPixelOut.normalMap = float4(normalMap.w, normalMap.y, normalMap.z, 1.0f);
#else
	albedoPixelOut.normalMap = float4(127.0f / 255.0f, 127.0f / 255.0f, 1.0f, alpha);
#endif
	albedoPixelOut.tangent = float4(tangent.xyz, 1.0f);

#ifdef ALBEDO_USE_NORMALMAP
	float specular = specMapTexture.Sample(specMapSampler, st).x;
	albedoPixelOut.specularGlowProperty.r = specular;
#endif
	albedoPixelOut.specularGlowProperty.a = alpha;

	return albedoPixelOut;
}