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
void RendererDrawPassLighting::Init()
{
	drawLightingConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(PS_DRAWLIGHTING_BUFFER), &drawLightingBuffer);
}

/*
=====================
RendererDrawPassLighting::Draw
=====================
*/
void RendererDrawPassLighting::Draw(const BuildRenderCommand &command)
{
	PolymerNGRenderTarget *drawWorldRenderTarget = renderer.GetWorldRenderTarget();

	//spos[0] = (float)tspr->y;
	//spos[1] = -(float)(tspr->z) / 16.0f;
	//spos[2] = -(float)tspr->x;

	rhi.SetShader(renderer.deferredLightingProgram->GetRHIShader());

	rhi.SetImageForContext(0, drawWorldRenderTarget->GetDiffuseImage(0)->GetRHITexture());
	rhi.SetImageForContext(1, drawWorldRenderTarget->GetDepthImage()->GetRHITexture());
	//rhi.SetImageForContext(1, imageManager.GetPaletteManager()->GetPaletteImage()->GetRHITexture());

	//drawLightingBuffer.invModelViewProjectionMatrix = command.taskDrawLights.inverseModelViewMatrix;
	//
	//drawLightingBuffer.lightposition[0] = command.taskDrawLights.visibleLights[0]->GetOpts()->position[1];
	//drawLightingBuffer.lightposition[1] = -(command.taskDrawLights.visibleLights[0]->GetOpts()->position[2] / 16.0f);
	//drawLightingBuffer.lightposition[2] = -command.taskDrawLights.visibleLights[0]->GetOpts()->position[0];

	drawLightingConstantBuffer->UpdateBuffer(&drawLightingBuffer, sizeof(PS_DRAWLIGHTING_BUFFER), 0);
	rhi.SetConstantBuffer(0, drawLightingConstantBuffer, false, true);
	rhi.DrawUnoptimized2DQuad(NULL);
}