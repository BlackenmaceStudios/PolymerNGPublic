// Renderer_Pass_DOF.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
=====================
RendererDrawPassDOF::Init
=====================
*/
void RendererDrawPassDOF::Init()
{
	BuildImage *diffuseRenderBuffer;
	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_RGBA8;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		diffuseRenderBuffer = new BuildImage(opts);
		diffuseRenderBuffer->UpdateImagePost(NULL);
	}
	renderTarget = new PolymerNGRenderTarget(diffuseRenderBuffer, NULL, NULL);
}

/*
=====================
RendererDrawPassDOF::Draw
=====================
*/
void RendererDrawPassDOF::Draw(const BuildRenderCommand &command)
{
	PolymerNGRenderTarget *drawPostProcessRenderTarget = renderer.GetPostProcessRenderTarget();
	renderTarget->Bind();
	rhi.SetShader(renderer.bokehDOFProgram->GetRHIShader());
	rhi.SetImageForContext(0, drawPostProcessRenderTarget->GetDiffuseImage(0)->GetRHITexture());
	rhi.SetImageForContext(1, renderer.GetWorldDepthBuffer()->GetRHITexture());
	rhi.DrawUnoptimized2DQuad(NULL);
	PolymerNGRenderTarget::BindDeviceBuffer();
}