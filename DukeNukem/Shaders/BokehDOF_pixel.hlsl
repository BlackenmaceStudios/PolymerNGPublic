#include "guishader.hlsli"
#include "Utility/ShaderUtility.hlsli"

Texture2D diffuseTexture : register(t0);
SamplerState diffuseTextureSampler : register(s0);

Texture2D depthTexture : register(t1);
SamplerState depthTextureSampler : register(s1);

//
// SampleFromColorBuffer
//
float4 SampleFromColorBuffer(float2 texcoord)
{
	return diffuseTexture.Sample(diffuseTextureSampler, texcoord);
}

//
// SampleFromDepthBuffer
//
float SampleFromDepthBuffer(float2 texcoord)
{
	float d = depthTexture.Sample(depthTextureSampler, texcoord).r;
	//return PSLinearizeDepthBuffer(d, 0.01, 300.0f);
	return d;
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

//------------------------------------------
//user variables

#define samples  3 //samples on the first ring
#define rings  5 //ring count

#define autofocus  true //use autofocus in shader? disable if you use external focalDepth value
#define focus  float2(0.5, 0.5) // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)
#define range  12.0 //focal range
#define maxblur  1.25 //clamp value of max blur

#define threshold  0.5 //highlight threshold;
#define gain  10.0 //highlight gain;

#define bias  0.4 //bokeh edge bias
#define fringe  0.5 //bokeh chromatic aberration/fringing

#define noise true //use noise instead of pattern for sample dithering
#define namount  0.0001 //dither amount

#define depthblur  true //blur the depth buffer?
#define dbsize  2.0 //depthblursize

					/*
					next part is experimental
					not looking good with small sample and ring count
					looks okay starting from samples = 4, rings = 4
					*/

#define pentagon  false //use pentagon as bokeh shape?
#define feather 0.4 //pentagon shape feather

					 //------------------------------------------


float penta(float2 coords) //pentagonal shape
{
	float scale = float(rings) - 1.3;
	float4  HS0 = float4(1.0, 0.0, 0.0, 1.0);
	float4  HS1 = float4(0.309016994, 0.951056516, 0.0, 1.0);
	float4  HS2 = float4(-0.809016994, 0.587785252, 0.0, 1.0);
	float4  HS3 = float4(-0.809016994, -0.587785252, 0.0, 1.0);
	float4  HS4 = float4(0.309016994, -0.951056516, 0.0, 1.0);
	float4  HS5 = float4(0.0, 0.0, 1.0, 1.0);

	float4  one = float4(1.0, 1.0, 1.0, 1.0);

	float4 P = float4((coords), float2(scale, scale));

	float4 dist = float4(0.0, 0.0, 0.0, 0.0);
	float inorout = -4.0;

	dist.x = dot(P, HS0);
	dist.y = dot(P, HS1);
	dist.z = dot(P, HS2);
	dist.w = dot(P, HS3);

	dist = smoothstep(-feather, feather, dist);

	inorout += dot(dist, one);

	dist.x = dot(P, HS4);
	dist.y = HS5.w - abs(P.z);

	dist = smoothstep(-feather, feather, dist);
	inorout += dist.x;

	return clamp(inorout, 0.0, 1.0);
}

float bdepth(float2 coords, float2 texel) //blurring depth
{
	float d = 0.0;
	float kernel[9];
	float2 offset[9];

	float2 wh = float2(texel.x, texel.y) * dbsize;

	offset[0] = float2(-wh.x, -wh.y);
	offset[1] = float2(0.0, -wh.y);
	offset[2] = float2(wh.x - wh.y, wh.x - wh.y);

	offset[3] = float2(-wh.x, 0.0);
	offset[4] = float2(0.0, 0.0);
	offset[5] = float2(wh.x, 0.0);

	offset[6] = float2(-wh.x, wh.y);
	offset[7] = float2(0.0, wh.y);
	offset[8] = float2(wh.x, wh.y);

	kernel[0] = 1.0 / 16.0;   kernel[1] = 2.0 / 16.0;   kernel[2] = 1.0 / 16.0;
	kernel[3] = 2.0 / 16.0;   kernel[4] = 4.0 / 16.0;   kernel[5] = 2.0 / 16.0;
	kernel[6] = 1.0 / 16.0;   kernel[7] = 2.0 / 16.0;   kernel[8] = 1.0 / 16.0;


	for (int i = 0; i<9; i++)
	{
		float tmp = SampleFromDepthBuffer(coords + offset[i]).r;
		d += tmp * kernel[i];
	}

	return d;
}


float3 color(float2 coords, float blur, float2 texel) //processing the sample
{
	float3 col = float3(0.0, 0.0, 0.0);

	col.r = SampleFromColorBuffer(coords + float2(0.0, 1.0)*texel*fringe*blur).r;
	col.g = SampleFromColorBuffer(coords + float2(-0.866, -0.5)*texel*fringe*blur).g;
	col.b = SampleFromColorBuffer(coords + float2(0.866, -0.5)*texel*fringe*blur).b;

	float3 lumcoeff = float3(0.299, 0.587, 0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum - threshold)*gain, 0.0);
	return col + lerp(float3(0.0, 0.0, 0.0), col, thresh*blur);
}

float2 rand(in float2 coord, float width, float height) //generating noise/pattern texture for dithering
{
	float noiseX = ((frac(1.0 - coord.x*(width / 2.0))*0.25) + (frac(coord.y*(height / 2.0))*0.75))*2.0 - 1.0;
	float noiseY = ((frac(1.0 - coord.x*(width / 2.0))*0.75) + (frac(coord.y*(height / 2.0))*0.25))*2.0 - 1.0;

	if (noise)
	{
		noiseX = clamp(frac(sin(dot(coord, float2(12.9898, 78.233))) * 43758.5453), 0.0, 1.0)*2.0 - 1.0;
		noiseY = clamp(frac(sin(dot(coord, float2(12.9898, 78.233)*2.0)) * 43758.5453), 0.0, 1.0)*2.0 - 1.0;
	}
	return float2(noiseX, noiseY);
}

#define PI  3.14159265
float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 resolution = GetColorBufferResolution();
	float2 texel = 1.0 / resolution;

	float depth = SampleFromDepthBuffer(input.texcoord0.xy).x;
	float blur = 0.0;

	if (depthblur)
	{
		depth = bdepth(input.texcoord0.xy, texel);
	}

	blur = clamp((abs(depth - 0.969) / range)*100.0, -maxblur, maxblur);

	//if (autofocus)
	//{
	//	float fDepth = SampleFromDepthBuffer(focus).x;
	//	blur = clamp((abs(depth - fDepth) / range)*100.0, -maxblur, maxblur);
	//}

	float2 noise2 = rand(input.texcoord0.xy, resolution.x, resolution.y)*namount*blur;

	float w = (1.0 / resolution.x)*blur + noise2.x;
	float h = (1.0 / resolution.y)*blur + noise2.y;

	float3 col = SampleFromColorBuffer(input.texcoord0.xy).rgb;
	float s = 1.0;

	int ringsamples;

	for (int i = 1; i <= rings; i += 1)
	{
		ringsamples = i * samples;

		for (int j = 0; j < ringsamples; j += 1)
		{
			float step = PI*2.0 / float(ringsamples);
			float pw = (cos(float(j)*step)*float(i));
			float ph = (sin(float(j)*step)*float(i));
			float p = 1.0;
			if (pentagon)
			{
				p = penta(float2(pw, ph));
			}
			col += color(input.texcoord0.xy + float2(pw*w, ph*h), blur, texel)*lerp(1.0, (float(i)) / (float(rings)), bias)*p;
			s += 1.0*lerp(1.0, (float(i)) / (float(rings)), bias)*p;
		}
	}


	col /= s;

	return float4(col.x, col.y, col.z, 1.0f);
}