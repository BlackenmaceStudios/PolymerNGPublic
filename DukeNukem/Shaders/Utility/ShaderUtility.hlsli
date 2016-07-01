// ShaderUtility.hlsli
//

//
// cotangent_frame 
// http://www.thetenthplanet.de/archives/1180
//
float3x3 cotangent_frame(float3 N, float3 p, float3 uv)
{
	// get edge vectors of the pixel triangle
	float3 dp1 = ddx(p);
	float3 dp2 = ddy(p);
	float3 duv1 = ddx(uv);
	float3 duv2 = ddy(uv);

	// solve the linear system
	float3 dp2perp = cross(dp2, N);
	float3 dp1perp = cross(N, dp1);
	float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame 
	float invmax = rsqrt(max(dot(T, T), dot(B, B)));
	return float3x3(T * invmax, B * invmax, N);
}

//
// Perturb_Normal 
// http://www.thetenthplanet.de/archives/1180
//
float3 perturb_normal(float3 map, float3 vnormal, float3 vposition, float2 vtexcoord)
{
#ifdef WITH_NORMALMAP_UNSIGNED
	map = map * 255. / 127. - 128. / 127.;
#endif
#ifdef WITH_NORMALMAP_2CHANNEL
	map.z = sqrt(1. - dot(map.xy, map.xy));
#endif
#ifdef WITH_NORMALMAP_GREEN_UP
	map.y = -map.y;
#endif

	float3x3 TBN = cotangent_frame(vnormal, -vposition, float3(vtexcoord.x, vtexcoord.y, 0));
	return normalize(mul(TBN, map));
}

//
// PSLinearizeDepthBuffer
//
float3 PSLinearizeDepthBuffer(float depth, float near, float far)
{
	return (2 * near) / (far + near - depth * (far - near));
}

//
// PSCalculateFastLowQualityDOF
//
float3 PSCalculateFastLowQualityDOF(float scale, float2 v_texCoord, Texture2D diffuseTexture, SamplerState diffuseTextureSampler)
{
	float2 v_blurTexCoords[14];
	float3 color = float3(0,0,0);
	v_blurTexCoords[0] = v_texCoord + (float2(0.0, -0.028) * scale);
	v_blurTexCoords[1] = v_texCoord + (float2(0.0, -0.024) * scale);
	v_blurTexCoords[2] = v_texCoord + (float2(0.0, -0.020) * scale);
	v_blurTexCoords[3] = v_texCoord + (float2(0.0, -0.016) * scale);
	v_blurTexCoords[4] = v_texCoord + (float2(0.0, -0.012) * scale);
	v_blurTexCoords[5] = v_texCoord + (float2(0.0, -0.008) * scale);
	v_blurTexCoords[6] = v_texCoord + (float2(0.0, -0.004) * scale);
	v_blurTexCoords[7] = v_texCoord + (float2(0.0, 0.004) * scale);
	v_blurTexCoords[8] = v_texCoord + (float2(0.0, 0.008) * scale);
	v_blurTexCoords[9] = v_texCoord + (float2(0.0, 0.012) * scale);
	v_blurTexCoords[10] = v_texCoord + (float2(0.0, 0.016) * scale);
	v_blurTexCoords[11] = v_texCoord + (float2(0.0, 0.020) * scale);
	v_blurTexCoords[12] = v_texCoord + (float2(0.0, 0.024) * scale);
	v_blurTexCoords[13] = v_texCoord + (float2(0.0, 0.028) * scale);

	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[0])*0.0044299121055113265;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[1])*0.00895781211794;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[2])*0.0215963866053;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[3])*0.0443683338718;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[4])*0.0776744219933;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[5])*0.115876621105;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[6])*0.147308056121;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_texCoord)*0.159576912161;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[7])*0.147308056121;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[8])*0.115876621105;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[9])*0.0776744219933;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[10])*0.0443683338718;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[11])*0.0215963866053;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[12])*0.00895781211794;
	color += diffuseTexture.Sample(diffuseTextureSampler, v_blurTexCoords[13])*0.0044299121055113265;
	return color;
}

// Function for converting depth to view-space position
// in deferred pixel shader pass.  vTexCoord is a texture
// coordinate for a full-screen quad, such that x=0 is the
// left of the screen, and y=0 is the top of the screen.
float3 VSPositionFromDepth(float z, float2 vTexCoord, float4x4 matInvProjection, float4x4 invViewMatrix)
{
	// Get x/w and y/w from the viewport position
	float x = vTexCoord.x * 2 - 1;
	float y = (1 - vTexCoord.y) * 2 - 1;
	float4 vProjectedPos = float4(x, y, z, 1.0f);
	// Transform by the inverse projection matrix
	float4 vPositionVS = mul(matInvProjection, vProjectedPos);
	// Divide by w to get the view-space position
	float3 viewSpacePosition = vPositionVS.xyz / vPositionVS.w;
	return mul(invViewMatrix, float4(viewSpacePosition, 1.0f)).xyz;
}
