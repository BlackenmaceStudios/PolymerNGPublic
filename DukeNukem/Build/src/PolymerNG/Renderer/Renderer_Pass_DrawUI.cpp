// Renderer_DrawUI.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
=====================
RendererDrawPassDrawUI::Init
=====================
*/
void RendererDrawPassDrawUI::Init()
{
	drawUIConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(PS_DRAWUI_BUFFER), &drawUIBuffer);
}

/*
=====================
RendererDrawPassDrawUI::Draw
=====================
*/
void RendererDrawPassDrawUI::Draw(const BuildRenderCommand &command)
{
//	BuildRHI::SetPSOForContext(Context, pso);
//	
	BuildRHIUIVertex vertexes[4];

	for (int i = 0; i < 4; i++)
	{
		vertexes[i].X = command.taskRotateSprite.vertexes[i].vertex.GetX();
		vertexes[i].Y = command.taskRotateSprite.vertexes[i].vertex.GetY();
		vertexes[i].Z = command.taskRotateSprite.vertexes[i].vertex.GetZ();
		vertexes[i].W = command.taskRotateSprite.vertexes[i].vertex.GetW();

		vertexes[i].U = command.taskRotateSprite.vertexes[i].textureCoords0.GetX();
		vertexes[i].V = command.taskRotateSprite.vertexes[i].textureCoords0.GetY();
		vertexes[i].U1 = command.taskRotateSprite.vertexes[i].vertex.GetZ();
		vertexes[i].U2 = command.taskRotateSprite.vertexes[i].vertex.GetW();
	}

	// Build doesn't submit the quad order properly, this is a hack to order it correctly.
	BuildRHIUIVertex corrected_vertexes[4];
	corrected_vertexes[0] = vertexes[3];
	corrected_vertexes[1] = vertexes[0];
	corrected_vertexes[2] = vertexes[2];
	corrected_vertexes[3] = vertexes[1];

	// For some reason we are either getting bashed memory, or somehting wierd is going on, either way check the texnum value so we don't crash.
	if (command.taskRotateSprite.texnum > MAXTILES)
	{
		initprintf("RendererDrawPassDrawUI::Draw: We have are trying to draw a tile that is out of bounds!\n");
		return;
	}

	BuildImage *image = polymerNG.GetImage(command.taskRotateSprite.texnum);
	if (image == NULL)
	{
		initprintf("RendererDrawPassDrawUI::Draw: Tried to draw a NULL image?\n");
		return;
	}
	if (image->GetRHITexture() == NULL)
	{
		return;
	}
	rhi.SetShader(renderer.ui_texture_basic->GetRHIShader());
	rhi.SetImageForContext(0, image->GetRHITexture());
	rhi.SetImageForContext(1, polymerNG.GetPaletteImage()->GetRHITexture());

	drawUIBuffer.modulationColor[0] = command.taskRotateSprite.spriteColor.GetX();
	drawUIBuffer.modulationColor[1] = command.taskRotateSprite.spriteColor.GetY();
	drawUIBuffer.modulationColor[2] = command.taskRotateSprite.spriteColor.GetZ();

	drawUIConstantBuffer->UpdateBuffer(&drawUIBuffer, sizeof(PS_DRAWUI_BUFFER), 0);
	rhi.SetConstantBuffer(0, drawUIConstantBuffer, false, true);
	rhi.DrawUnoptimized2DQuad(corrected_vertexes);
}