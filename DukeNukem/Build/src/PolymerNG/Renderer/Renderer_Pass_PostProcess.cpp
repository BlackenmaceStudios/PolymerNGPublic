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

	rhi.SetShader(renderer.postProcessProgram->GetRHIShader());

	rhi.SetImageForContext(0, drawWorldRenderTarget->GetDiffuseImage(0)->GetRHITexture());
	rhi.SetImageForContext(1, hdrLightingBuffer->GetDiffuseImage(0)->GetRHITexture());
	rhi.SetImageForContext(2, drawWorldRenderTarget->GetDiffuseImage(2)->GetRHITexture());
	rhi.SetImageForContext(3, drawWorldRenderTarget->GetDepthImage()->GetRHITexture());

	drawPostProcessBuffer.viewMatrix = command.taskDrawLights.viewMatrix;
	drawPostProcessBuffer.invModelViewProjectionMatrix = command.taskDrawLights.inverseModelViewMatrix;


	drawPostProcessConstantBuffer->UpdateBuffer(&drawPostProcessBuffer, sizeof(PS_POSTPROCESS_BUFFER), 0);
	rhi.SetConstantBuffer(0, drawPostProcessConstantBuffer, SHADER_BIND_PIXELSHADER);
	rhi.DrawUnoptimized2DQuad(NULL);
}