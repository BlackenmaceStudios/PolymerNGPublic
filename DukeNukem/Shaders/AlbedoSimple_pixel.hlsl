#include "AlbedoSimple.hlsli"
#include "Utility/ShaderUtility.hlsli"

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4 shadeOffsetVisibility;
	float4 fogDensistyScaleEnd;
	float4 fogColor;
	float4 normal;
	float4 tangent;
	float4 ambient;
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

#ifdef GLOWMAP
Texture2D glowMapTexture : register(t5);
SamplerState glowMapSampler : register(s5);
#endif

#ifdef FAKE_TRANSPARENT
	#ifndef ALBEDO_USE_NORMALMAP
		Texture2D normalMapTexture : register(t3);
		SamplerState normalMapSampler : register(s3);
	#endif
#endif

#ifdef FAKE_TRANSPARENT
Texture2D prevSceneMapTexture : register(t6);
SamplerState prevSceneMapSampler : register(s6);
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
#ifndef FAKE_TRANSPARENT
	float depth : SV_DEPTH;
#endif
	float4 diffuseColor			: SV_Target0;
	float4 surfaceNormal		: SV_Target1;
	float4 normalMap			: SV_Target2;
	float4 tangent				: SV_Target3;
	float4 specularGlowProperty : SV_Target4;
	float4 ambient				: SV_Target5;
	float4 glowColorBuffer		: SV_Target6;
};

#ifdef FAKE_TRANSPARENT
float2 GetScreenspaceCoordinate (VertexShaderOutput input)
{
	float width, height, levels;
	prevSceneMapTexture.GetDimensions(0, width, height, levels);
	float2 st = float2(input.position.x / width, input.position.y / height);
	return st;
}
#endif

#ifdef FAKE_TRANSPARENT
float4 GetRefractedColor(float4 normalMap, VertexShaderOutput input)
{
	float2 screenSpaceCoord = GetScreenspaceCoordinate(input);

	float2 refractedUV = screenSpaceCoord + normalMap.xy * 0.01;
	return prevSceneMapTexture.Sample(prevSceneMapSampler, refractedUV);
}

float Fresnel(float NdotL, float fresnelBias, float fresnelPow)
{
	float facing = (1.0 - NdotL);

	return max(fresnelBias + (1.0 - fresnelBias) * pow(facing, fresnelPow), 0.0);
}

float4 GetReflectedColor(VertexShaderOutput input)
{
	//float4 normalMap = 2.0 * normalMapTexture.Sample(normalMapSampler, input.texcoord0.xy) - 1.0;
	//float LdotN = dot(normalMap.xyz, input.eyeposition.xyz);
	//
	//half3 vReflect = 2.0 * LdotN * normalMap.xyz - input.eyeposition.xyz;
	//
	//float2 screenSpaceCoord = GetScreenspaceCoordinate(input);
	//
	//float2 refractedUV = screenSpaceCoord + normalMap.xy * 0.02;
	//refractedUV.y = 1.0 - refractedUV.y;
	//return prevSceneMapTexture.Sample(prevSceneMapSampler, refractedUV * 0.005);
	return float4(1, 1, 1, 1);
}
#endif

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
	
#ifdef FAKE_TRANSPARENT
	float4 normalMap = 2.0 * normalMapTexture.Sample(normalMapSampler, input.texcoord0.xy) - 1.0;
	float4 refractedColor = GetRefractedColor(normalMap, input);

	float NdotL = max(dot(input.eyeposition.xyz, normalMap.xyz), 0);
	float facing = (1.0 - NdotL);
	float fresnel = Fresnel(NdotL, 0.2, 5.0);

	float4 waterResult = refractedColor * fresnel;
	albedoPixelOut.diffuseColor = float4(waterResult.x, waterResult.y, waterResult.z, 1.0);
#else
	albedoPixelOut.diffuseColor = float4(result.x, result.y, result.z, alpha);
	#ifdef GLOW
		#ifdef GLOWMAP
			albedoPixelOut.glowColorBuffer.xyz = glowMapTexture.Sample(glowMapSampler, st).xyz;
			// Only disable lighting if the glowmap has a pixel that would actually glow.
			if (albedoPixelOut.glowColorBuffer.x > 0 && albedoPixelOut.glowColorBuffer.y > 0 && albedoPixelOut.glowColorBuffer.z > 0)
			{
				albedoPixelOut.specularGlowProperty.y = 1.0f; // This is only temporary!
			}
			albedoPixelOut.glowColorBuffer.a = alpha;
		#else
			albedoPixelOut.specularGlowProperty.y = 1.0f; // This is only temporary!
			albedoPixelOut.glowColorBuffer = albedoPixelOut.diffuseColor;
		#endif
	#endif

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

		albedoPixelOut.ambient = ambient / 255;
		albedoPixelOut.ambient.a = alpha;
#endif

	return albedoPixelOut;
}
