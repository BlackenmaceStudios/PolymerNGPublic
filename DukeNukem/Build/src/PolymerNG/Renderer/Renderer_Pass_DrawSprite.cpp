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
		BuildImage *image = static_cast<BuildImage *>(sprite->plane.renderImageHandle);
		drawSpriteBuffer.mWorldViewProj = sprite->modelViewProjectionMatrix;
		drawSpriteBuffer.modelMatrix = sprite->modelMatrix;
		//drawSpriteBuffer.modelMatrix = sprite->modelMatrix;
		drawSpriteConstantBuffer->UpdateBuffer(&drawSpriteBuffer, sizeof(VS_DRAWSPRITE_BUFFER), 0);

		rhi.SetConstantBuffer(0, drawSpriteConstantBuffer);

		psDrawSpriteBuffer.shadeOffsetVisibility[0] = sprite->plane.shadeNum;
		psDrawSpriteBuffer.shadeOffsetVisibility[1] = sprite->plane.visibility;
		psDrawSpriteBuffer.fogDensistyScaleEnd[0] = sprite->plane.fogDensity;
		psDrawSpriteBuffer.fogDensistyScaleEnd[1] = sprite->plane.fogStart;
		psDrawSpriteBuffer.fogDensistyScaleEnd[2] = sprite->plane.fogEnd;
		drawSpritePixelConstantBuffer->UpdateBuffer(&psDrawSpriteBuffer, sizeof(PS_CONSTANT_BUFFER), 0);
		rhi.SetConstantBuffer(0, drawSpritePixelConstantBuffer, false, true);
		if (image == NULL || image->GetRHITexture() == NULL)
		{
			continue;
		}
		rhi.SetImageForContext(0, image->GetRHITexture());
		rhi.SetImageForContext(1, polymerNG.GetPaletteImage()->GetRHITexture());
		rhi.SetImageForContext(2, polymerNG.GetPaletteLookupImage(sprite->paletteNum)->GetRHITexture());
		if (sprite->isHorizsprite)
		{
			rhi.DrawUnoptimizedQuad(renderer.spriteSimpleProgram->GetRHIShader(), spriteRHIMesh, 4, 4);
		}
		else
		{
			rhi.DrawUnoptimizedQuad(renderer.spriteSimpleProgram->GetRHIShader(), spriteRHIMesh, 0, 4);
		}
	}
}