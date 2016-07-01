#include "ShadowShared.hlsli"
#include "VertexShaderShared.hlsli"

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix mWorldView;
	float4 zFarzNearAndDir;
};

//
// I used the code from gamedevelop - jmarshall
// http://gamedevelop.eu/en/tutorials/dual-paraboloid-shadow-mapping.htm
//
VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput Out;

	// transform vertex to DP-space
	Out.Position = mul(mWorldView, float4(input.position.xyz, 1.0f));
	Out.Position /= Out.Position.w;

	// for the back-map z has to be inverted
	Out.Position.z *= zFarzNearAndDir.z;

	// because the origin is at 0 the proj-vector
	// matches the vertex-position
	float fLength = length(Out.Position.xyz);

	// normalize
	Out.Position /= fLength;

	// save for clipping 	
	Out.texcoord0.x = Out.Position.z;

	// calc "normal" on intersection, by adding the 
	// reflection-vector(0,0,1) and divide through 
	// his z to get the texture coords
	Out.Position.x /= Out.Position.z + 1.0f;
	Out.Position.y /= Out.Position.z + 1.0f;

	// set z for z-buffering and neutralize w
	Out.Position.z = (fLength - zFarzNearAndDir.y) / (zFarzNearAndDir.x - zFarzNearAndDir.y);
	Out.Position.w = 1.0f;

	Out.texcoord0.y = Out.Position.z;
	Out.texcoord0.z = Out.texcoord0.w = 1;

	return Out;
}