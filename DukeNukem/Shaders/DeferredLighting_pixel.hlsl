#include "guishader.hlsli"
#include "Utility/ShaderUtility.hlsli"

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
	float4x4 invModelViewProjectionMatrix;
	float4 lightposition;
	float4 cameraposition;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

Texture2D depthTexture : register(t1);
SamplerState depthTextureSampler : register(s1);


float3 ComputeLightFactor(float3 lightposition, float3 fragmentposition)
{
	float3 lightDir = normalize(lightposition - fragmentposition);

	float3 normal = float3(80.0f / 255.0f, 80.0f / 255.0f, 1.0f); // Neutral normal color.
	return max(dot(normal, lightDir), 0);
}

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 st = input.texcoord0.xy;

	// Grab the color from the diffuse render target. If alpha == 0(sky) then discard.
	float4 color = diffuseTexture.Sample(diffuseTextureSampler, st);
	if (color.a == 0)
		discard;

	// Grab the depth a compute the position from the depth buffer.
	float depth = depthTexture.Sample(depthTextureSampler, st);
	float3 position = VSPositionFromDepth(depth, st, invModelViewProjectionMatrix);
	
	// Set the default color.
	float3 currentcolor = float3(1, 1, 1);

	// Compute the light factor.
	for (int i = 0; i < 1; i++)
	{
		currentcolor = currentcolor * ComputeLightFactor(lightposition.xyz, position.xyz);
	}

	currentcolor = color.xyz;

	return float4(currentcolor.x, currentcolor.y, currentcolor.z, color.a);
}