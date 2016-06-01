// Renderer_Draw_ DrawSprite.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
=====================
RendererDrawPassDrawSprite::Init
=====================
*/
void RendererDrawPassDrawSprite::Init()
{
	Build3DVertex spriteVertexes[8];

	// Vertical sprites.
	spriteVertexes[1].SetVertex(Build3DVector4(-0.5f, 0.0f, 0.0f, 1.0f), Build3DVector4(0.0f, 1.0f, 0.0f, 0.0f));
	spriteVertexes[0].SetVertex(Build3DVector4(0.5f, 0.0f, 0.0f, 1.0f), Build3DVector4(1.0f, 1.0f, 0.0f, 0.0f));
	spriteVertexes[2].SetVertex(Build3DVector4(0.5f, 1.0f, 0.0f, 1.0f), Build3DVector4(1.0f, 0.0f, 0.0f, 0.0f));
	spriteVertexes[3].SetVertex(Build3DVector4(-0.5f, 1.0f, 0.0f, 1.0f), Build3DVector4(0.0f, 0.0f, 0.0f, 0.0f));

	// Hortizal sprites.
	spriteVertexes[5].SetVertex(Build3DVector4(-0.5f, 0.0f, 0.5f, 1.0f), Build3DVector4(0.0f, 0.0f, 0.0f, 0.0f));
	spriteVertexes[4].SetVertex(Build3DVector4(0.5f, 0.0f, 0.5f, 1.0f), Build3DVector4(1.0f, 0.0f, 0.0f, 0.0f));
	spriteVertexes[6].SetVertex(Build3DVector4(0.5f, 0.0f, -0.5f, 1.0f), Build3DVector4(1.0f, 1.0f, 0.0f, 0.0f));
	spriteVertexes[7].SetVertex(Build3DVector4(-0.5f, 0.0f, -0.5f, 1.0f), Build3DVector4(0.0f, 1.0f, 0.0f, 0.0f));

	spriteRHIMesh = rhi.AllocateRHIMesh(sizeof(Build3DVertex), 8, &spriteVertexes[0], false);

	drawSpriteConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_DRAWSPRITE_BUFFER), &drawSpriteBuffer);
	drawSpritePixelConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(PS_DRAWSPRITE_BUFFER), &psDrawSpriteBuffer);
}

/*
=====================
RendererDrawPassDrawSprite::Draw
=====================
*/
void RendererDrawPassDrawSprite::Draw(const BuildRenderCommand &command)
{
	for (int i = 0; i < command.taskRenderSprites.numSprites; i++)
	{
		Build3DSprite *sprite = &command.taskRenderSprites.prsprites[i];
		
		if (!sprite->isVisible)
			continue;

		PolymerNGMaterial *material = static_cast<PolymerNGMaterial *>(sprite->plane.renderMaterialHandle);
		drawSpriteBuffer.mWorldViewProj = sprite->modelViewProjectionMatrix;
		drawSpriteBuffer.modelMatrix = sprite->modelMatrix;
		//drawSpriteBuffer.modelMatrix = sprite->modelMatrix;
		drawSpriteConstantBuffer->UpdateBuffer(&drawSpriteBuffer, sizeof(VS_DRAWSPRITE_BUFFER), 0);
		rhi.SetConstantBuffer(0, drawSpriteConstantBuffer);

		if (sprite->cacheModel)
		{
			for (int d = 0; d < sprite->cacheModel->GetNumSurfaces(); d++)
			{
				CacheModelSurface *surface = sprite->cacheModel->GetCacheSurface(d);

				if (surface->material == NULL || surface->material->GetDiffuseTexture() == NULL || sprite->cacheModel->GetBaseModel() == NULL)
					continue;
				
				rhi.SetImageForContext(0, surface->material->GetDiffuseTexture()->GetRHITexture());

				BuildRHIShader *shader = renderer.albedoHQProgram->GetRHIShader();

				rhi.DrawIndexedQuad(shader, sprite->cacheModel->GetBaseModel()->rhiVertexBufferStatic, 0, surface->startIndex, surface->numIndexes);
			}
		}
		else
		{
			psDrawSpriteBuffer.shadeOffsetVisibility[0] = sprite->plane.shadeNum;
			psDrawSpriteBuffer.shadeOffsetVisibility[1] = sprite->plane.visibility;
			psDrawSpriteBuffer.fogDensistyScaleEnd[0] = sprite->plane.fogDensity;
			psDrawSpriteBuffer.fogDensistyScaleEnd[1] = sprite->plane.fogStart;
			psDrawSpriteBuffer.fogDensistyScaleEnd[2] = sprite->plane.fogEnd;
			drawSpritePixelConstantBuffer->UpdateBuffer(&psDrawSpriteBuffer, sizeof(PS_CONSTANT_BUFFER), 0);
			rhi.SetConstantBuffer(0, drawSpritePixelConstantBuffer, false, true);
			if (material->GetDiffuseTexture() == NULL || material->GetDiffuseTexture()->GetRHITexture() == NULL)
			{
				continue;
			}
			rhi.SetImageForContext(0, material->GetDiffuseTexture()->GetRHITexture());
			rhi.SetImageForContext(1, imageManager.GetPaletteManager()->GetPaletteImage()->GetRHITexture());
			rhi.SetImageForContext(2, imageManager.GetPaletteManager()->GetPaletteLookupImage(sprite->paletteNum)->GetRHITexture());

			BuildRHIShader *shader = renderer.albedoSimpleProgram->GetRHIShader();
			if (material->GetDiffuseTexture()->GetOpts().isHighQualityImage)
				shader = renderer.spriteHQProgram->GetRHIShader();

			if (sprite->isHorizsprite)
			{
				rhi.DrawUnoptimizedQuad(shader, spriteRHIMesh, 4, 4);
			}
			else
			{
				rhi.DrawUnoptimizedQuad(shader, spriteRHIMesh, 0, 4);
			}
		}
	}
}