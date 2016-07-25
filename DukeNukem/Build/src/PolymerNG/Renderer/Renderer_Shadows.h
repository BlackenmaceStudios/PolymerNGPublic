// Renderer_Shadows.h
//

#pragma once

#define NUM_QUEUED_SHADOW_MAPS 30
#define SHADOW_MAP_SIZE		512

//
// VS_SHADOW_POINT_CONSTANT_BUFFER
//
struct VS_SHADOW_POINT_CONSTANT_BUFFER
{
	float4x4 mWorldView;
	float4x4 mModelMatrix;
	float4 light_position_and_range;
};

//
// ShadowMap
//
struct ShadowMap
{
	PolymerNGRenderTarget *shadowMapCubeMap;
	PolymerNGRenderTarget *spotLightMap;
};
