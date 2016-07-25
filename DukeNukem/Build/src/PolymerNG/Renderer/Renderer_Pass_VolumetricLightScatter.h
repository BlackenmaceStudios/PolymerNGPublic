#pragma once

struct PS_DRAWVLS_BUFFER
{
	float4x4 invModelViewProjectionMatrix;
	float4x4 invViewMatrix;
	float4 CameraPos;
	float4 LightPosAndRadius;
	float4 LightColor;
};

class RendererDrawPassVolumetricLightScatter : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:
	PS_DRAWVLS_BUFFER			psDrawVLSBuffer;
	PolymerNGRenderTarget *		renderTarget;
	BuildRHIConstantBuffer *	vlsConstantBufferPixel;
};
