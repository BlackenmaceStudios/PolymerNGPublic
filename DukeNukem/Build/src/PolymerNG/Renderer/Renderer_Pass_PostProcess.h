// Renderer_Pass_PostProcess.h
//

#pragma once

struct PS_POSTPROCESS_BUFFER
{
	float4x4 invModelViewProjectionMatrix;
	float4x4 viewMatrix;
};

class RendererDrawPassPostProcess : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);

	PolymerNGRenderTarget		*GetPostProcessRenderTarget() { return renderTarget; }

	BuildImage					*GetPreviousFrameImage() { return diffuseRenderBufferCopy; }
private:
	PS_POSTPROCESS_BUFFER		drawPostProcessBuffer;
	BuildRHIConstantBuffer		*drawPostProcessConstantBuffer;
	PolymerNGRenderTarget		*renderTarget;
	BuildImage					*diffuseRenderBufferCopy;
};
