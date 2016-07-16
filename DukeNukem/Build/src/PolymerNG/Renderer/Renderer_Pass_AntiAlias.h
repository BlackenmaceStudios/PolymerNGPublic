// Renderer_Pass_AntiAlias.h
//

#pragma once

struct PS_DRAWAA_BUFFER
{
	float4x4 invModelViewProjectionMatrix;
	float4x4 viewMatrix;
};

class RendererDrawPassAA : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:

};
