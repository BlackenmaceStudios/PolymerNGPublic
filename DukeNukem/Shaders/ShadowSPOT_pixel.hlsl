#include "ShadowShared.hlsli"

struct PS_OUTPUT
{
	float4 color : SV_TARGET;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

PS_OUTPUT main(VertexShaderOutput pixelin)
{
	PS_OUTPUT output;

#ifdef USE_ALPHA_SAMPLE
	// Grab the color from the diffuse render target, if not 1 discard.
	float4 color = diffuseTexture.Sample(diffuseTextureSampler, pixelin.diffuseTexCoords);
	if (color.a != 1)
		discard;
#endif

	//// get distance between fragment and light source
	//float lightDistance = length(pixelin.texcoord0.xyz - pixelin.lightpos_and_range.xyz);
	//
	//// map to [0;1] range by dividing by far_plane
	//lightDistance = lightDistance / pixelin.lightpos_and_range.w;

	output.color = float4(0, 0, 0, 0);
	//output.depth = lightDistance;
	return output;
}