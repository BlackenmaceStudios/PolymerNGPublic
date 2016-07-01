// Renderer_Pass_PostProcess.h
//

#pragma once

struct PS_POSTPROCESS_BUFFER
{
	Math::XMFLOAT4X4 invModelViewProjectionMatrix;
	Math::XMFLOAT4X4 viewMatrix;
};

class RendererDrawPassPostProcess : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:
	PS_POSTPROCESS_BUFFER		drawPostProcessBuffer;
	BuildRHIConstantBuffer		*drawPostProcessConstantBuffer;
};
