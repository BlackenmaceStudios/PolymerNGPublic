#include "ShadowShared.hlsli"

struct PS_OUTPUT
{
	float4 color : SV_TARGET;
	float depth : SV_DEPTH;
};

PS_OUTPUT main(VertexShaderOutput pixelin)
{
	PS_OUTPUT output;

	// get distance between fragment and light source
	float lightDistance = length(pixelin.texcoord0.xyz - pixelin.lightpos_and_range.xyz);

	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / pixelin.lightpos_and_range.w;

	output.color = float4(0, 0, 0, 0);
	output.depth = lightDistance;
	return output;
}