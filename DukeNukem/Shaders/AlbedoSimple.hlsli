struct VertexShaderOutput
{
	float4 position  : SV_POSITION;
	float2 texcoord0 : TEXCOORD0;
	float3 texcoord1 : TEXCOORD1;
	float3 texcoord2 : TEXCOORD2;
	float3 texcoord3 : TEXCOORD3;
	float4 vDepthVS  : TEXCOORD4;
#ifdef FAKE_TRANSPARENT
	float4 eyeposition : TEXCOORD5;
#endif
};
