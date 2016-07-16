#include "guishader.hlsli"

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

//
// SampleFromColorBuffer
//
float4 SampleFromColorBuffer(float2 texcoord)
{
	return diffuseTexture.Sample(diffuseTextureSampler, texcoord);
}

//
// GetColorBufferResolution
//
float2 GetColorBufferResolution()
{
	float width, height, levels;
	diffuseTexture.GetDimensions(0, width, height, levels);
	return float2(width, height);
}

/**
Basic FXAA implementation based on the code on geeks3d.com with the
modification that the texture2DLod stuff was removed since it's
unsupported by WebGL.

--

From:
https://github.com/mitsuhiko/webgl-meincraft

Copyright (c) 2011 by Armin Ronacher.

Some rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided
with the distribution.

* The names of the contributors may not be used to endorse or
promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FXAA_REDUCE_MIN
#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#endif
#ifndef FXAA_SPAN_MAX
#define FXAA_SPAN_MAX     8.0
#endif

//optimized version for mobile, where dependent 
//texture reads can be a bottleneck
float4 fxaa(float2 fragCoord, float2 resolution, float2 v_rgbNW, float2 v_rgbNE, float2 v_rgbSW, float2 v_rgbSE, float2 v_rgbM) 
{
	float4 color;
	float2 inverseVP = float2(1.0 / resolution.x, 1.0 / resolution.y);
	float3 rgbNW = SampleFromColorBuffer(v_rgbNW).xyz;
	float3 rgbNE = SampleFromColorBuffer(v_rgbNE).xyz;
	float3 rgbSW = SampleFromColorBuffer(v_rgbSW).xyz;
	float3 rgbSE = SampleFromColorBuffer(v_rgbSE).xyz;
	float4 texColor = SampleFromColorBuffer(v_rgbM);
	float3 rgbM = texColor.xyz;
	float3 luma = float3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM = dot(rgbM, luma);
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	float2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
		(0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
		max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
			dir * rcpDirMin)) * inverseVP;

	float3 rgbA = 0.5 * (
		SampleFromColorBuffer(fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
		SampleFromColorBuffer(fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
	float3 rgbB = rgbA * 0.5 + 0.25 * (
		SampleFromColorBuffer(fragCoord * inverseVP + dir * -0.5).xyz +
		SampleFromColorBuffer(fragCoord * inverseVP + dir * 0.5).xyz);

	float lumaB = dot(rgbB, luma);
	if ((lumaB < lumaMin) || (lumaB > lumaMax))
		color = float4(rgbA, texColor.a);
	else
		color = float4(rgbB, texColor.a);
	return color;
}

//To save 9 dependent texture reads, you can compute
//these in the vertex shader and use the optimized
//frag.glsl function in your frag shader. 

//This is best suited for mobile devices, like iOS.

void fxaa_texcoords(float2 fragCoord, float2 resolution, out float2 v_rgbNW, out float2 v_rgbNE, out float2 v_rgbSW, out float2 v_rgbSE, out float2 v_rgbM) 
{
	float2 inverseVP = 1.0 / resolution.xy;
	v_rgbNW = (fragCoord + float2(-1.0, -1.0)) * inverseVP;
	v_rgbNE = (fragCoord + float2(1.0, -1.0)) * inverseVP;
	v_rgbSW = (fragCoord + float2(-1.0, 1.0)) * inverseVP;
	v_rgbSE = (fragCoord + float2(1.0, 1.0)) * inverseVP;
	v_rgbM = float2(fragCoord * inverseVP);
}

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 v_rgbNW;
	float2 v_rgbNE;
	float2 v_rgbSW;
	float2 v_rgbSE;
	float2 v_rgbM;

	float2 resolution = GetColorBufferResolution();

	float2 st = input.texcoord0.xy * resolution;

	//compute the texture coords
	fxaa_texcoords(st, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);

	//compute FXAA
	return fxaa(st, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
}