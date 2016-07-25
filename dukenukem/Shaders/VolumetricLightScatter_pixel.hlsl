/*
=============================================================================
VolumetricLightScatter_pixel.hlsl Volumetric Light Scattering Effect
Ported from my Unreal Engine 4 plugin:
https://raw.githubusercontent.com/jmarshall23/UnrealEngine/release/Engine/Shaders/PostProcessVolumetricLightScatter.usf?token=AGrUbvxwPhCguXSg-M3lRtHHinJ5D5wMks5XnO1FwA%3D%3D

by Justin Marshall
=============================================================================
*/

#include "guishader.hlsli"
#include "Utility/ShaderUtility.hlsli"

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4x4 invModelViewProjectionMatrix;
	float4x4 invViewMatrix;
	float4 CameraPos;
	float4 LightPosAndRadius;
	float4 LightColor;
};

Texture2D depthTexture : register(t0);
SamplerState depthTextureSampler : register(s0);

TextureCube light0ShadowMap : register(t1);
SamplerState light0ShadowMapSampler : register(s1);

//
// SampleFromDepthBuffer
//
float4 SampleFromDepthBuffer(float2 texcoord)
{
	return depthTexture.Sample(depthTextureSampler, texcoord);
}

//
// ComputeShadowFactor
//
float ComputeShadowFactor(float3 position, float3 lightPosition, float sceneDepth, float range)
{
	position *= (1.0f / 1000.0f);
	lightPosition *= (1.0f / 1000.0f);

	// Get vector between fragment position and light position
	float3 fragToLight = position - lightPosition;
	fragToLight.z = -fragToLight.z;

	// Use the light to fragment vector to sample from the depth map    
	float closestDepth = light0ShadowMap.Sample(light0ShadowMapSampler, fragToLight).r;
	closestDepth *= range;

	float currentDepth = length(fragToLight);
	float shadow = currentDepth - 0.05 < closestDepth ? 1.0 : 0.0;

	return shadow;
}


float intersectSphere(float3 rayOrigin, float3 rayDirection, float3 sphereOrigin, float sphereRadius)
{
	float3 oc = rayOrigin - sphereOrigin;
	float b = 2.0 * dot(rayDirection, oc);
	float c = dot(oc, oc) - sphereRadius*sphereRadius;
	float disc = b * b - 4.0 * c;

	if (disc < 0.0)
		return -1.0;

	// compute q as described above
	float q;
	if (b < 0.0)
		q = (-b - sqrt(disc)) / 2.0;
	else
		q = (-b + sqrt(disc)) / 2.0;

	float t0 = q;
	float t1 = c / q;

	// make sure t0 is smaller than t1
	if (t0 > t1) {
		// if t0 is bigger than t1 swap them around
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}

	// if t1 is less than zero, the object is in the ray's negative direction
	// and consequently the ray misses the sphere
	if (t1 < 0.0)
		return -1.0;

	// if t0 is less than zero, the intersection point is at t1
	if (t0 < 0.0) {
		return t1;
	}
	else {
		return t0;
	}
}

float4 main(VertexShaderOutput input) : SV_TARGET
{ 
	float2 st = input.texcoord0.xy;

	// Grab the depth a compute the position from the depth buffer.
	float SceneDepth = depthTexture.Sample(depthTextureSampler, st);

	// Grab the position from the depth.
	float3 WorldPos = VSPositionFromDepth(SceneDepth, st, invModelViewProjectionMatrix, invViewMatrix);

	float3 rayVector = WorldPos - CameraPos;

	int VLS_STEPS = 200;
	float rayLength = length(rayVector);
	float3 rayDirection = rayVector / rayLength;

	if (intersectSphere(CameraPos, rayDirection, LightPosAndRadius.xyz, LightPosAndRadius.w * 1000) == -1)
	{
		discard;
	}

	float stepLength = rayLength / VLS_STEPS;

	float3 step = rayDirection * stepLength;

	float3 currentPosition = CameraPos;

	float3 fog = float3(0.0f, 0.0f, 0.0f);
	float attenuation = 0;

	float lightRadius = LightPosAndRadius.w * 1000;
	for (int i = 0; i < VLS_STEPS; i++)
	{
		float shadowMapValue = ComputeShadowFactor(currentPosition, LightPosAndRadius.xyz, SceneDepth, lightRadius);
		if (shadowMapValue > 0.0)
		{
			float3 lightDirection = LightPosAndRadius.xyz - currentPosition.xyz;
			float dist = length(lightDirection);

			attenuation += 1.0 - ((dist * dist) / (lightRadius * lightRadius));
			fog += 1;
		}

		currentPosition += step;
	}

	float3 fogFinal = (fog * (attenuation / VLS_STEPS)) / VLS_STEPS; // (fog * (attenuation)) / VLS_STEPS;
	fogFinal *= 5;

	float4 OutColor;
	OutColor.xyz = clamp(fogFinal * 1.0f * normalize(LightColor), 0.0f, 1.0f); //Texture2DSample(PostprocessInput0, PostprocessInput0Sampler, InUV);
	OutColor.w = 1;
	return OutColor;
}