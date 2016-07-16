// ShadowShared.hlsli
//

struct VertexShaderOutput
{
	float4 Position  : SV_POSITION;
	float4 texcoord0 : TEXCOORD0;
	float4 lightpos_and_range  : TEXCOORD1;
	float4 diffuseTexCoords : TEXCOORD2;
};

struct GeometryShaderOutput
{
	float4 Position  : SV_POSITION;
	uint slice : SV_RenderTargetArrayIndex;
	float4 texcoord0 : TEXCOORD0;
};