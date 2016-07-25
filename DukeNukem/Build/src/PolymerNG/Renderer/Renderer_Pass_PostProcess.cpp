// Renderer_Pass_Lighting.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
=====================
RendererDrawPassLighting::Init
=====================
*/
void RendererDrawPassPostProcess::Init()
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
	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_RGBA8;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		diffuseRenderBufferCopy = new BuildImage(opts);
		diffuseRenderBufferCopy->UpdateImagePost(NULL);
	}
	renderTarget = new PolymerNGRenderTarget(diffuseRenderBuffer, NULL, NULL);
	drawPostProcessConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(PS_POSTPROCESS_BUFFER), &drawPostProcessBuffer);
}

/*
=====================
RendererDrawPassLighting::Draw
=====================
*/
void RendererDrawPassPostProcess::Draw(const BuildRenderCommand &command)
{
	PolymerNGRenderTarget *drawWorldRenderTarget = renderer.GetWorldRenderTarget();
	PolymerNGRenderTarget *hdrLightingBuffer = renderer.GetHDRLightingRenderTarget();

	if (!renderer.GetNextPageParms().shouldSkipDOFAndAA)
	{
		renderTarget->Bind();
	}

	rhi.SetShader(renderer.postProcessProgram->GetRHIShader());

	rhi.SetImageForContext(0, drawWorldRenderTarget->GetDiffuseImage(0)->GetRHITexture());
	rhi.SetImageForContext(1, hdrLightingBuffer->GetDiffuseImage(0)->GetRHITexture());
	rhi.SetImageForContext(2, drawWorldRenderTarget->GetDiffuseImage(2)->GetRHITexture());
	rhi.SetImageForContext(3, drawWorldRenderTarget->GetDepthImage()->GetRHITexture());
	rhi.SetImageForContext(4, drawWorldRenderTarget->GetDiffuseImage(5)->GetRHITexture());
	rhi.SetImageForContext(5, drawWorldRenderTarget->GetDiffuseImage(6)->GetRHITexture());

	drawPostProcessBuffer.viewMatrix = command.taskDrawLights.viewMatrix;
	drawPostProcessBuffer.invModelViewProjectionMatrix = command.taskDrawLights.inverseModelViewMatrix;


	drawPostProcessConstantBuffer->UpdateBuffer(&drawPostProcessBuffer, sizeof(PS_POSTPROCESS_BUFFER), 0);
	rhi.SetConstantBuffer(0, drawPostProcessConstantBuffer, SHADER_BIND_PIXELSHADER);
	rhi.DrawUnoptimized2DQuad(NULL);
	rhi.CopyDepthToAnotherImage(renderTarget->GetDiffuseImage(0)->GetRHITexture(), diffuseRenderBufferCopy->GetRHITexture());
	PolymerNGRenderTarget::BindDeviceBuffer();
}