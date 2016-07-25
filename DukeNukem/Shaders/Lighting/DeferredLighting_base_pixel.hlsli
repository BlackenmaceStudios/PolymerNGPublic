#include "../guishader.hlsli"
#include "../Utility/ShaderUtility.hlsli"

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4x4 invModelViewProjectionMatrix;
	float4x4 viewMatrix;
	float4 lightposition_and_range;
	float4 lightcolorrange;
	float4 cameraposition;
	float4 lightPositionUnknown;

	float4x4 lightViewMatrix;
	float4x4 lightProjectionMatrix;
	float4	 lightbrightnessandunknown;
	float4x4 invViewMatrix;
	float4 spotDir;
	float4 spotRadius;
};

#define USE_SLOW_POSITION_METHOD

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

Texture2D normalTexture : register(t1);
SamplerState normalTextureSampler : register(s1);

Texture2D normalMapTexture : register(t2);
SamplerState normalMapTextureSampler : register(s2);

Texture2D tangentTexture : register(t3);
SamplerState tangentTextureSampler : register(s3);

Texture2D specularGlowPropertyTexture : register(t4);
SamplerState specularGlowPropertyTextureSampler : register(s4);

Texture2D depthTexture : register(t5);
SamplerState depthTextureSampler : register(s5);


#ifndef NOSHADOWS
	#if POINTLIGHT
		TextureCube light0ShadowMap : register(t6);
		SamplerState light0ShadowMapSampler : register(s6);
	#elif SPOTLIGHT
		Texture2D light0ShadowMap : register(t6);
		SamplerState light0ShadowMapSampler : register(s6);
	#endif
#endif

#if !NOSHADOWS
	#if POINTLIGHT
		float ComputeShadowFactor(float3 position, float3 lightPosition, float sceneDepth, float range, float2 offset)
		{
			position *= (1.0f / 1000.0f);
			lightPosition *= (1.0f / 1000.0f);

			// Get vector between fragment position and light position
			float3 fragToLight = position - lightPosition;
			fragToLight.z = -fragToLight.z;

			// Use the light to fragment vector to sample from the depth map    
			float closestDepth = light0ShadowMap.Sample(light0ShadowMapSampler, fragToLight + (float3(offset, 1.0) * (1.0f / 1024.0f))).r;
			closestDepth *= range;

			float currentDepth = length(fragToLight);
			float shadow = currentDepth - 0.05 < closestDepth ? 1.0 : 0.0;

			return shadow;
		}
	#elif SPOTLIGHT
		float ComputeShadowFactor(float3 position, float3 lightPosition, float sceneDepth, float range, float2 offset)
		{
			position *= (1.0f / 1000.0f);
			lightPosition *= (1.0f / 1000.0f);

			// Get vector between fragment position and light position
			float4 fragToLight = mul((lightProjectionMatrix * lightViewMatrix), float4(position.x, position.y, position.z, 1.0));

			// Use the light to fragment vector to sample from the depth map    
			float shadowdepth = light0ShadowMap.Sample(light0ShadowMapSampler, fragToLight.xy + (float3(offset, 1.0) * (1.0f / 512.0f))).r;

			if (shadowdepth < fragToLight.z)
				return 0;

			return 1;
		}
	#endif
#endif

float3 ComputeLightFactor(float3 normal, float3 lightposition, float3 fragmentposition)
{
	float3 lightDir = normalize(lightposition - fragmentposition);
	return max(dot(normal, lightDir), 0);
}

float3 DirectIllumination(float3 pos, float3 norm, float3 lightposition, float lightradius, float3 lightcolor, float inSpec, float3x3 TBN)
{
	float3 lightPos = lightposition;

	float3 lightVec = mul(TBN, lightPos - pos);

	float d = length(lightVec);
	lightradius = lightradius * 1000;
	if (d > lightradius)
	{
		return float3(0, 0, 0);
	}

	float att = pow(max(0.0f, 1.0 - (d / lightradius)), 2.0f);

	float3 lightVecScaled = lightVec / d;
	float diffuseFactor = dot(lightVecScaled, norm);

	if (diffuseFactor < 0.0f)
	{
		return float3(0, 0, 0);
	}


	float3 halfAngleVector = mul(TBN, (lightPos - pos) + (cameraposition - pos));
	float3 hDotN = dot(normalize(halfAngleVector), norm);
	float specFactor = pow(hDotN, 1.0f) * inSpec;
#if POINTLIGHT
	float spotAttenuation = 1;
#elif SPOTLIGHT
	float3 D = normalize(spotDir.xyz);
	float spotCosAngle = dot(-lightVecScaled, D);
	float spotAttenuation = clamp((spotCosAngle - spotRadius.x) * spotRadius.y, 0.0, 1.0);
#endif
	return (lightcolor *att * (diffuseFactor) * spotAttenuation) + (lightcolor * specFactor * (att * 7) );
}

float3 DirectIlluminationSprite(float3 pos, float3 norm, float3 lightposition, float lightradius, float3 lightcolor, float inSpec)
{
	float3 lightPos = lightposition;

	float3 lightVec = lightPos - pos;

	float d = length(lightVec);
	//lightradius = lightradius * 1000;
	if (d > lightradius)
	{
		return float3(0, 0, 0);
	}

	float att = pow(max(0.0f, 1.0 - (d / lightradius)), 2.0f);
	return lightcolor * att;
}





//#define USE_SLOW_POSITION_METHOD
float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 st = input.texcoord0.xy;

	// Grab the depth a compute the position from the depth buffer.
	float depth = depthTexture.Sample(depthTextureSampler, st);
	float linearDepth = PSLinearizeDepthBuffer(depth, 0.01f, 300.0f);

	// Grab the position from the depth.
	float3 position = VSPositionFromDepth(depth, st, invModelViewProjectionMatrix, invViewMatrix);

	// Determine if this light actually effects this pixel.
	float3 lightVec = lightposition_and_range.xyz - position;
	if (length(lightVec) > lightcolorrange.w * 1000)
	{
		discard;
	}

	// Grab the color from the diffuse render target. If alpha == 0(sky) then discard.
	float4 color = diffuseTexture.Sample(diffuseTextureSampler, st);
	if (color.a == 0)
		discard;

	// Construct the tangent binormal, normal matrix.
	float4 normal = normalTexture.Sample(normalTextureSampler, st);
	float4 tangent = tangentTexture.Sample(tangentTextureSampler, st);
	float3 binormal = cross(tangent.xyz, normal.xyz);
	float3x3 TBN = float3x3(normalize(tangent.xyz), normalize(binormal.xyz), normalize(normal.xyz));
	//TBN = transpose(TBN);

	float3 normal_map = normalMapTexture.Sample(normalMapTextureSampler, st).xyz;
	normal_map = normalize(2.0 * (normal_map - 0.5));
	//normal_map = mul(TBN, normal_map);

	
	//float3 position = positionTexture.Sample(positionTextureSampler, st);

	float3 specularGlowProperty = specularGlowPropertyTexture.Sample(specularGlowPropertyTextureSampler, st).xyz;
	
	float4 positionInViewSpace = mul(viewMatrix, float4(position.xyz, 1.0));

	// Set the default color.
	float3 lightContribution = float3(0, 0, 0);

	// Compute the light factor.
	float3 lightPositionInViewSpace = mul(viewMatrix, float4(lightposition_and_range.xyz, 1.0)).xyz;
#ifdef NOSHADOWS
	float shadowFactor = 1;
#else
	float shadowFactor = 0;
	{
		float x, y;
		for (y = -1.5; y <= 1.5; y += 1.0)
			for (x = -1.5; x <= 1.5; x += 1.0)
				shadowFactor += ComputeShadowFactor(position.xyz, lightposition_and_range.xyz, linearDepth, lightposition_and_range.w, float2(x, y));

		shadowFactor /= 16.0;
	}
	//shadowFactor = max(shadowFactor, 0.1);
#endif
	//float3 DirectIllumination(float3 pos, float3 norm, float3 lightposition, float lightradius, float3 lightcolor, float inSpec)
	float3 lightfactor = 0;

	if (specularGlowProperty.y == 1)
	{
		lightfactor = float3(1, 1, 1);
	}
	else if (specularGlowProperty.z == 1)
	{
		lightfactor = shadowFactor * DirectIlluminationSprite(positionInViewSpace.xyz, normal, lightPositionInViewSpace, lightcolorrange.w, lightcolorrange.xyz, 0);
		lightfactor *= lightbrightnessandunknown.x;
	}
	else
	{
		lightfactor = shadowFactor * DirectIllumination(position.xyz, normal_map, lightposition_and_range.xyz, lightcolorrange.w, lightcolorrange.xyz, specularGlowProperty.x, TBN);
		lightfactor *= lightbrightnessandunknown.x;
	}
	
	lightContribution = lightContribution + lightfactor;  //ComputeLightFactor(normal, mul(viewMatrix, float4(lightposition.xyz, 1.0)).xyz, position.xyz);


	float3 finalColor = (lightContribution);


	return float4(finalColor.x, finalColor.y, finalColor.z, 1);
}