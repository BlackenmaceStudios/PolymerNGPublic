// Renderer_Pass_Lighting.h
//

#pragma once

struct PS_DRAWLIGHTING_BUFFER
{
	Math::XMFLOAT4X4 invModelViewProjectionMatrix;
	float lightposition[4];
	float cameraposition[4];
};

class RendererDrawPassLighting : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:
	PS_DRAWLIGHTING_BUFFER		drawLightingBuffer;
	BuildRHIConstantBuffer		*drawLightingConstantBuffer;
};
