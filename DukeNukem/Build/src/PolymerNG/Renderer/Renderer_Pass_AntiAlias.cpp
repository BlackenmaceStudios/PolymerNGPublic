// Renderer_Pass_AntiAlias.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
=====================
RendererDrawPassAA::Init
=====================
*/
void RendererDrawPassAA::Init()
{

}

/*
=====================
RendererDrawPassAA::Draw
=====================
*/
void RendererDrawPassAA::Draw(const BuildRenderCommand &command)
{
	PolymerNGRenderTarget *renderTarget = renderer.GetDOFRenderTarget();

	rhi.SetShader(renderer.fxaaProgram->GetRHIShader());
	rhi.SetImageForContext(0, renderTarget->GetDiffuseImage(0)->GetRHITexture());
	rhi.DrawUnoptimized2DQuad(NULL);
}