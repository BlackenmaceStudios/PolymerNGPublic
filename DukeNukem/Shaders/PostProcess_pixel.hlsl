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

Texture2D ambientTexture : register(t4);
SamplerState ambientTextureSampler : register(s4);

Texture2D glowTexture : register(t5);
SamplerState glowTextureSampler : register(s5);

//
// GetColorBufferResolution
//
float2 GetColorBufferResolution()
{
	float width, height, levels;
	diffuseTexture.GetDimensions(0, width, height, levels);
	return float2(width, height);
}

float3 GaussianBlur(Texture2D tex, SamplerState samplerState, float2 centreUV, float2 pixelOffset) {
	float3 colOut = float3(0.0, 0.0, 0.0);

	const int stepCount = 9;
	float gWeights[stepCount];
	gWeights[0] = 0.10855;
	gWeights[1] = 0.13135;
	gWeights[2] = 0.10406;
	gWeights[3] = 0.07216;
	gWeights[4] = 0.04380;
	gWeights[5] = 0.02328;
	gWeights[6] = 0.01083;
	gWeights[7] = 0.00441;
	gWeights[8] = 0.00157;
	float gOffsets[stepCount];
	gOffsets[0] = 0.66293;
	gOffsets[1] = 2.47904;
	gOffsets[2] = 4.46232;
	gOffsets[3] = 6.44568;
	gOffsets[4] = 8.42917;
	gOffsets[5] = 10.41281;
	gOffsets[6] = 12.39664;
	gOffsets[7] = 14.38070;
	gOffsets[8] = 16.36501;

	for (int i = 0; i < stepCount; i++) {
		float2 texCoordOffset = gOffsets[i] * pixelOffset;
		float3 col = tex.Sample(samplerState, centreUV + texCoordOffset).xyz + glowTexture.Sample(glowTextureSampler, centreUV - texCoordOffset).xyz;
		colOut += gWeights[i] * col;
	}
	return colOut;
}


#define USE_SLOW_POSITION_METHOD
float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 st = input.texcoord0.xy;

	// Grab the color from the diffuse render target. If alpha == 0(sky) then discard.
	float4 color = diffuseTexture.Sample(diffuseTextureSampler, st);
	if (color.a == 0)
		discard;

	// Grab the color from the diffuse render target. If alpha == 0(sky) then discard.
	float4 ambient = ambientTexture.Sample(ambientTextureSampler, st);
	ambient = max(ambient, 0.05);
	float3 hdrLighting = lightTexture.Sample(lightTextureSampler, st).xyz;
	float3 finalColor = color.xyz * ((ambient.xyz * 2 ) + hdrLighting.xyz);

	float2 step = 1.0 / GetColorBufferResolution();
	float3 glowresult = GaussianBlur(glowTexture, glowTextureSampler, float2(st.x, st.y), float2(step.x*1.5, 0));

	finalColor = finalColor + glowresult.xyz;


	const float gamma = 1.1;
	const float exposure = 2;
	float3 tonedresult = float3(1.0, 1.0, 1.0) - exp(-finalColor * exposure);
	// also gamma correct while we're at it       
	tonedresult = pow(tonedresult, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));


	return float4(tonedresult.x, tonedresult.y, tonedresult.z, 1);
}