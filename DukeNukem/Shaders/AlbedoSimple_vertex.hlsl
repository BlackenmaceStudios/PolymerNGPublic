#include "AlbedoSimple.hlsli"
#include "VertexShaderShared.hlsli"

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix mWorldViewProjection;
	matrix mWorldView;
	matrix mWorldViewInverse;
	matrix mProjection;
	float4 viewpositionanddepthoffset;
};


VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 vertex = mul(mWorldViewProjection, float4(input.position.xyz, 1.0));
	output.position = vertex;
	output.texcoord0 = input.texcoord0;
	output.texcoord1 = input.position.xyz; // mul(mWorldView, float4(input.position.xyz, 1.0));
	output.texcoord2 = mul(transpose(mWorldViewInverse), float4(input.normal.xyz, 1.0));
	output.texcoord3 = viewpositionanddepthoffset.xyz - input.position.xyz;
#ifdef FAKE_TRANSPARENT
	output.eyeposition.xyz = viewpositionanddepthoffset.xyz;
#endif

	float4 vertex_world = mul(mWorldView, float4(input.position.xyz, 1.0));
	float4 vertex_screen = mul(mProjection, float4(input.position.xyz, 1.0));

	output.vDepthVS = vertex;

	return output;
}
