// Renderer_Pass_Lighting.h
//

#pragma once

#define MAX_SHADOW_MAPS				3

//
// ShadowMapInfo
//
struct ShadowMapInfo
{
	PolymerNGRenderTarget			*slices[6];
};

struct PS_DRAWLIGHTING_BUFFER
{
	float4x4 invModelViewProjectionMatrix;
	float4x4 viewMatrix;
	float lightposition_and_range[4];
	float lightrangcolor[4];
	float4 cameraposition;
	float numLightsUnkown[4];
	float4x4 lightViewMatrix;
	float4x4 lightProjectionMatrix;
	float4 lightbrightnessandunknown;
	float4x4 inverseViewMatrix;
	float4 spotDir;
	float4 spotRadius;
};

class RendererDrawPassLighting : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);

	PolymerNGRenderTarget		*GetHDRLightingBuffer() { return renderTarget; }

	bool shouldClear;
private:
	PS_DRAWLIGHTING_BUFFER		drawLightingBuffer;
	BuildRHIConstantBuffer		*drawLightingConstantBuffer;

	PolymerNGRenderTarget		*renderTarget;

	ShadowMapInfo				shadowMapQueue[MAX_SHADOW_MAPS];
};
