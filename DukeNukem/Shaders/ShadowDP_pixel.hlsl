#include "ShadowShared.hlsli"

struct PS_OUTPUT
{
	float4 color : SV_TARGET;
	float depth : SV_DEPTH;
};

PS_OUTPUT main(VertexShaderOutput pixelin) 
{
	PS_OUTPUT output;
	clip(pixelin.texcoord0.x);

	output.color = float4(1, 1, 1, 1);
	output.depth = pixelin.texcoord0.y;
	return output;
}