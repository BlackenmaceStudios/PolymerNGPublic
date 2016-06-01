// Renderer_Pass_DrawWorld.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
========================
RendererDrawPassDrawWorld::Init
========================
*/
void RendererDrawPassDrawWorld::Init()
{
	BuildImage *diffuseRenderBuffer;
	BuildImage *glowRenderBuffer;
	BuildImage *depthRenderBuffer;

	drawWorldConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_DRAWWORLD_BUFFER), &drawWorldBuffer);
	drawWorldPixelConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(PS_CONSTANT_BUFFER), &drawWorldPixelBuffer);

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_RGBA32;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		diffuseRenderBuffer = new BuildImage(opts);
		diffuseRenderBuffer->UpdateImagePost(NULL);
	}

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_RGBA32;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		glowRenderBuffer = new BuildImage(opts);
		glowRenderBuffer->UpdateImagePost(NULL);
	}

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_DEPTH;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = false;

		depthRenderBuffer = new BuildImage(opts);
		depthRenderBuffer->UpdateImagePost(NULL);
	}

	renderTarget = new PolymerNGRenderTarget(diffuseRenderBuffer, depthRenderBuffer, NULL);
	renderTarget->AddRenderTaret(glowRenderBuffer);
}

/*
========================
RendererDrawPassDrawWorld::DrawPlane
========================
*/
void RendererDrawPassDrawWorld::DrawPlane(BuildRHIMesh *rhiMesh, const BaseModel *model, const Build3DPlane *plane)
{
	PolymerNGMaterial *material = static_cast<PolymerNGMaterial *>(plane->renderMaterialHandle);

	if (material != NULL)
	{
		//drawWorldPixelBuffer.shadeOffsetVisibility[0] = plane->shadeNum;
		//drawWorldPixelBuffer.shadeOffsetVisibility[1] = plane->visibility;
		//drawWorldPixelBuffer.fogColor[0] = plane->fogColor[0];
		//drawWorldPixelBuffer.fogColor[1] = plane->fogColor[1];
		//drawWorldPixelBuffer.fogColor[2] = plane->fogColor[2];
		//
		//drawWorldPixelBuffer.fogDensistyScaleEnd[0] = plane->fogDensity;
		//drawWorldPixelBuffer.fogDensistyScaleEnd[1] = plane->fogStart;
		//drawWorldPixelBuffer.fogDensistyScaleEnd[2] = plane->fogEnd;

		

		rhi.SetImageForContext(0, material->GetDiffuseTexture()->GetRHITexture());
		rhi.SetImageForContext(1, imageManager.GetPaletteManager()->GetPaletteImage()->GetRHITexture());
		rhi.SetImageForContext(2, imageManager.GetPaletteManager()->GetPaletteLookupImage(plane->paletteNum)->GetRHITexture());

		BuildRHIShader *shader = renderer.albedoSimpleProgram->GetRHIShader();
		if (material->GetDiffuseTexture()->GetOpts().isHighQualityImage)
			shader = renderer.albedoHQProgram->GetRHIShader();

		if (plane->ibo_offset != -1)
		{
			rhi.DrawIndexedQuad(shader, rhiMesh, 0, plane->ibo_offset, plane->indicescount);
		}
		else
		{
			rhi.DrawUnoptimizedQuad(shader, rhiMesh, plane->vbo_offset, plane->vertcount);
		}
	}
}
/*
========================
RendererDrawPassDrawWorld::BindDrawWorldRenderTarget
========================
*/
void RendererDrawPassDrawWorld::BindDrawWorldRenderTarget(bool enable)
{
	if(enable)
		renderTarget->Bind();
	else
		PolymerNGRenderTarget::BindDeviceBuffer();
}

/*
========================
RendererDrawPassDrawWorld::Draw
========================
*/
void RendererDrawPassDrawWorld::Draw(const BuildRenderCommand &command)
{
	
	drawWorldBuffer.mWorldViewProj = command.taskRenderWorld.viewProjMatrix;
	drawWorldConstantBuffer->UpdateBuffer(&drawWorldBuffer, sizeof(VS_DRAWWORLD_BUFFER), 0);

	rhi.SetConstantBuffer(0, drawWorldConstantBuffer);

	const Build3DBoard *board = command.taskRenderWorld.board;

	int currentSMPFrame = renderer.GetCurrentFrameNum();

	drawWorldPixelConstantBuffer->UpdateBuffer(&drawWorldPixelBuffer, sizeof(PS_CONSTANT_BUFFER), 0);
	rhi.SetConstantBuffer(0, drawWorldPixelConstantBuffer, false, true);

	// Render all of the static sectors.
	for (int i = 0; i < command.taskRenderWorld.numRenderPlanes; i++)
	{
		DrawPlane(board->GetBaseModel()->rhiVertexBufferStatic, board->GetBaseModel(), command.taskRenderWorld.renderplanes[i]);
	}
}