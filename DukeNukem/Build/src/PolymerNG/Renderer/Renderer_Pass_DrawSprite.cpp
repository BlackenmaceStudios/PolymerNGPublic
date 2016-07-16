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
	// Vertical sprites.
	spriteVertexes[0].SetVertex(Build3DVector4(-0.5f, 0.0f, 0.0f, 1.0f), Build3DVector4(0.0f, 1.0f, 0.0f, 0.0f));
	spriteVertexes[1].SetVertex(Build3DVector4(0.5f, 0.0f, 0.0f, 1.0f), Build3DVector4(1.0f, 1.0f, 0.0f, 0.0f));
	spriteVertexes[2].SetVertex(Build3DVector4(0.5f, 1.0f, 0.0f, 1.0f), Build3DVector4(1.0f, 0.0f, 0.0f, 0.0f));
	spriteVertexes[3].SetVertex(Build3DVector4(-0.5f, 1.0f, 0.0f, 1.0f), Build3DVector4(0.0f, 0.0f, 0.0f, 0.0f));

	// Hortizal sprites.
	spriteVertexes[4].SetVertex(Build3DVector4(-0.5f, 0.0f, 0.5f, 1.0f), Build3DVector4(0.0f, 0.0f, 0.0f, 0.0f));
	spriteVertexes[5].SetVertex(Build3DVector4(0.5f, 0.0f, 0.5f, 1.0f), Build3DVector4(1.0f, 0.0f, 0.0f, 0.0f));
	spriteVertexes[6].SetVertex(Build3DVector4(0.5f, 0.0f, -0.5f, 1.0f), Build3DVector4(1.0f, 1.0f, 0.0f, 0.0f));
	spriteVertexes[7].SetVertex(Build3DVector4(-0.5f, 0.0f, -0.5f, 1.0f), Build3DVector4(0.0f, 1.0f, 0.0f, 0.0f));

#if !POLYMERNG_NOSYNC_SPRITES
	spriteRHIMesh = rhi.AllocateRHIMesh(sizeof(Build3DVertex), 8, &spriteVertexes[0], false);
#else
	spriteRHIMesh = rhi.AllocateRHIMesh(sizeof(Build3DVertex), 8, &spriteVertexes[0], true);
#endif

	sprite3DPlanes[SPRITE_FACING_VERTICAL].buffer = &spriteVertexes[0];
	sprite3DPlanes[SPRITE_FACING_VERTICAL].vertcount = 4;
	sprite3DPlanes[SPRITE_FACING_VERTICAL].vbo_offset = 0;
	
	sprite3DPlanes[SPRITE_FACING_HORIZONTAL].buffer = &spriteVertexes[4];
	sprite3DPlanes[SPRITE_FACING_HORIZONTAL].vertcount = 4;
	sprite3DPlanes[SPRITE_FACING_HORIZONTAL].vbo_offset = 4;

	unsigned int indexes[12] = { 0, 1, 2, 3, 0, 2, 4, 5, 6, 7, 4, 6 };

	// Vertical sprites.
	{
		sprite3DPlanes[SPRITE_FACING_VERTICAL].indices = new unsigned short[6];
		for (int i = 0; i < 6; i++)
			sprite3DPlanes[SPRITE_FACING_VERTICAL].indices[i] = indexes[i];

		sprite3DPlanes[SPRITE_FACING_VERTICAL].ibo_offset = 0;
		sprite3DPlanes[SPRITE_FACING_VERTICAL].indicescount = 6;
	}

	// Horizontal sprites.
	{
		sprite3DPlanes[SPRITE_FACING_HORIZONTAL].indices = new unsigned short[6];
		for (int i = 0; i < 6; i++)
			sprite3DPlanes[SPRITE_FACING_HORIZONTAL].indices[i] = indexes[i+6];
		sprite3DPlanes[SPRITE_FACING_HORIZONTAL].ibo_offset = 6;
		sprite3DPlanes[SPRITE_FACING_HORIZONTAL].indicescount = 6;
	}

	rhi.AllocateRHIMeshIndexes(spriteRHIMesh, 12, &indexes[0], false);

	//computeplane(&sprite3DPlanes[SPRITE_FACING_VERTICAL]);

	drawSpriteConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_DRAWSPRITE_BUFFER), &drawSpriteBuffer);
	drawSpritePixelConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(PS_DRAWSPRITE_BUFFER), &psDrawSpriteBuffer);
}

/*
=====================
RendererDrawPassDrawSprite::CalculateTransformAndNormalsForSprite
=====================
*/
void RendererDrawPassDrawSprite::CalculateTransformAndNormalsForSprite(SpriteFacingType spriteType, float4x4 spriteModelMatrix)
{
	Build3DVertex *gpuVertexes = &spriteVertexesGPU[sprite3DPlanes[spriteType].vbo_offset];
	for (int i = 0; i < 4; i++)
	{
		float4 position = float4(gpuVertexes[i].position.x, gpuVertexes[i].position.y, gpuVertexes[i].position.z, 1);
		float4 final_position = position * spriteModelMatrix;
		gpuVertexes[i].position.x = final_position.x;
		gpuVertexes[i].position.y = final_position.y;
		gpuVertexes[i].position.z = final_position.z;
		gpuVertexes[i].position.w = final_position.w;
	}

	sprite3DPlanes[spriteType].buffer = &spriteVertexesGPU[sprite3DPlanes[spriteType].vbo_offset];
	computeplane(&sprite3DPlanes[spriteType]);
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
		drawSpriteBuffer.modelMatrixInverse = sprite->modelViewInverse;
		drawSpriteBuffer.viewPosition[0] = -command.taskRenderSprites.position.x;
		drawSpriteBuffer.viewPosition[1] = -command.taskRenderSprites.position.y;
		drawSpriteBuffer.viewPosition[2] = -command.taskRenderSprites.position.z;

		if (sprite->isWallSprite || sprite->isHorizsprite)
		{
			drawSpriteBuffer.viewPosition[3] = 0.0001f;
		}
		else
		{
			drawSpriteBuffer.viewPosition[3] = 0.0f;
		}

		float4x4 spriteModelMatrix;
		memcpy(&spriteModelMatrix, &sprite->modelMatrix, sizeof(float) * 16);

		//drawSpriteBuffer.modelMatrix = sprite->modelMatrix;
		drawSpriteConstantBuffer->UpdateBuffer(&drawSpriteBuffer, sizeof(VS_DRAWSPRITE_BUFFER), 0);
		rhi.SetConstantBuffer(0, drawSpriteConstantBuffer, SHADER_BIND_VERTEXSHADER);

		if (sprite->cacheModel)
		{
			for (int d = 0; d < sprite->cacheModel->GetNumSurfaces(); d++)
			{
				CacheModelSurface *surface = sprite->cacheModel->GetCacheSurface(d);

				if (surface->material == NULL || sprite->cacheModel->GetBaseModel() == NULL)
					continue;
				
				if (surface->material->GetDiffuseTexture() == NULL)
				{
					rhi.SetImageForContext(0, imageManager.GetBlackImage()->GetRHITexture());
				}
				else
				{
					rhi.SetImageForContext(0, surface->material->GetDiffuseTexture()->GetRHITexture());
				}

				BuildRHIShader *shader = renderer.albedoHQNoNormalMapProgram->GetRHIShader();

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
			psDrawSpriteBuffer.ambient[0] = sprite->ambientColor[0];
			psDrawSpriteBuffer.ambient[1] = sprite->ambientColor[1];
			psDrawSpriteBuffer.ambient[2] = sprite->ambientColor[2];
			drawSpritePixelConstantBuffer->UpdateBuffer(&psDrawSpriteBuffer, sizeof(PS_CONSTANT_BUFFER), 0);
			rhi.SetConstantBuffer(0, drawSpritePixelConstantBuffer, SHADER_BIND_PIXELSHADER);
			if (material->GetDiffuseTexture() == NULL || material->GetDiffuseTexture()->GetRHITexture() == NULL)
			{
				continue;
			}
			rhi.SetImageForContext(0, material->GetDiffuseTexture()->GetRHITexture());
			rhi.SetImageForContext(1, imageManager.GetPaletteManager()->GetPaletteImage()->GetRHITexture());
			rhi.SetImageForContext(2, imageManager.GetPaletteManager()->GetPaletteLookupImage(sprite->paletteNum)->GetRHITexture());

			BuildRHIShader *shader = NULL;
			if (material->GetDiffuseTexture()->GetOpts().isHighQualityImage)
			{
				if (G_IsGlowSprite(sprite->plane.tileNum))
				{
					shader = renderer.spriteHQGlowProgram->GetRHIShader();
				}
				else
				{
					shader = renderer.spriteHQProgram->GetRHIShader();
				}
			}
			else
			{
				if (material->GetGlowMap() != NULL)
				{
					shader = renderer.spriteSimpleGlowMapProgram->GetRHIShader();
					rhi.SetImageForContext(5, material->GetGlowMap()->GetRHITexture());
				}
				else if (G_IsGlowSprite(sprite->plane.tileNum))
				{
					shader = renderer.spriteSimpleGlowProgram->GetRHIShader();
				}
				else
				{
					shader = renderer.spriteSimpleProgram->GetRHIShader();
				}
			}

			// Calculate the normals for the sprite
			memcpy(&spriteVertexesGPU[0], &spriteVertexes[0], sizeof(Build3DVertex) * 8);

			if (sprite->isHorizsprite)
			{
#if POLYMERNG_NOSYNC_SPRITES
				CalculateTransformAndNormalsForSprite(SPRITE_FACING_HORIZONTAL, spriteModelMatrix);
				rhi.UpdateRHIMesh(spriteRHIMesh, sprite3DPlanes[SPRITE_FACING_HORIZONTAL].vbo_offset, sizeof(Build3DVertex), sprite3DPlanes[SPRITE_FACING_HORIZONTAL].vertcount, sprite3DPlanes[SPRITE_FACING_HORIZONTAL].buffer);
#endif
				rhi.DrawIndexedQuad(shader, spriteRHIMesh, 0, sprite3DPlanes[SPRITE_FACING_HORIZONTAL].ibo_offset, sprite3DPlanes[SPRITE_FACING_HORIZONTAL].indicescount);
			}
			else
			{
#if POLYMERNG_NOSYNC_SPRITES
				CalculateTransformAndNormalsForSprite(SPRITE_FACING_VERTICAL, spriteModelMatrix);
				rhi.UpdateRHIMesh(spriteRHIMesh, sprite3DPlanes[SPRITE_FACING_VERTICAL].vbo_offset, sizeof(Build3DVertex), sprite3DPlanes[SPRITE_FACING_VERTICAL].vertcount, sprite3DPlanes[SPRITE_FACING_VERTICAL].buffer);
#endif
				rhi.DrawIndexedQuad(shader, spriteRHIMesh, 0, sprite3DPlanes[SPRITE_FACING_VERTICAL].ibo_offset, sprite3DPlanes[SPRITE_FACING_VERTICAL].indicescount);
			}
		}
	}
}