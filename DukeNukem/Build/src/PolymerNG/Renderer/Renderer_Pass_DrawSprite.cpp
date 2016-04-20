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
		// HACK! Hide the first person view sprite!!!!
		if (sprite->plane.tileNum == 1405)
			continue;
		BuildImage *image = static_cast<BuildImage *>(sprite->plane.renderImageHandle);
		drawSpriteBuffer.mWorldViewProj = sprite->modelViewProjectionMatrix;
		//drawSpriteBuffer.modelMatrix = sprite->modelMatrix;
		drawSpriteConstantBuffer->UpdateBuffer(&drawSpriteBuffer, sizeof(VS_DRAWSPRITE_BUFFER), 0);

		rhi.SetConstantBuffer(0, drawSpriteConstantBuffer);

		rhi.SetImageForContext(0, image->GetRHITexture());
		rhi.SetImageForContext(1, polymerNG.GetPaletteImage()->GetRHITexture());
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