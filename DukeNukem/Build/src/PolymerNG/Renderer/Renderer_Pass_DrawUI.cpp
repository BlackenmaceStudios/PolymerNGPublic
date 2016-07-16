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
	drawVSUIConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_DRAWUI_BUFFER), &drawVSUIBuffer);
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

	float screenRatio = globalWindowWidth / globalWindowHeight;

	// This is stupid!!!!
	if(globalWindowWidth > 1920 || globalWindowHeight > 1080)
		screenRatio = screenRatio - 0.3;

	// For some reason when this was inlined in the shader [3,4] and [4, 3] were swapped, inorder for this to work this has to be the same.
	float projectionMatrixBuild[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, screenRatio, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0001f, 1.0f,
		0.0f, 0.0f, -0.0001f, 0.0f
	};
	float projectionMatrixOrtho[16];
	memset(projectionMatrixOrtho, 0, sizeof(float) * 16);
	projectionMatrixOrtho[0] = 2.0f / globalWindowWidth;
	projectionMatrixOrtho[12] = -1.0f;
	projectionMatrixOrtho[5] = -2.0f / globalWindowHeight;
	projectionMatrixOrtho[13] = 1.0f;
	projectionMatrixOrtho[10] = 1.0f;
	projectionMatrixOrtho[15] = 1.0f;

	for (int i = 0; i < 4; i++)
	{
		vertexes[i].X = command.taskRotateSprite.vertexes[i].vertex.x;
		vertexes[i].Y = command.taskRotateSprite.vertexes[i].vertex.y;
		vertexes[i].Z = command.taskRotateSprite.vertexes[i].vertex.z;
		vertexes[i].W = command.taskRotateSprite.vertexes[i].vertex.w;

		vertexes[i].U = command.taskRotateSprite.vertexes[i].textureCoords0.x;
		vertexes[i].V = command.taskRotateSprite.vertexes[i].textureCoords0.y;
		vertexes[i].U1 = command.taskRotateSprite.vertexes[i].vertex.z;
		vertexes[i].U2 = command.taskRotateSprite.vertexes[i].vertex.w;
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

	PolymerNGMaterial *material = static_cast<PolymerNGMaterial *>(command.taskRotateSprite.renderMaterialHandle);
	if (material == NULL)
	{
		initprintf("RendererDrawPassDrawUI::Draw: Tried to draw a NULL image?\n");
		return;
	}
	if (material->GetDiffuseTexture()->GetRHITexture() == NULL)
	{
		return;
	}

	if (material->GetDiffuseTexture()->GetOpts().isHighQualityImage || command.taskRotateSprite.forceHQShader)
	{
		rhi.SetShader(renderer.ui_texture_hq_basic->GetRHIShader());
	}
	else
	{
		rhi.SetShader(renderer.ui_texture_basic->GetRHIShader());
	}
	rhi.SetImageForContext(0, material->GetDiffuseTexture()->GetRHITexture());
	rhi.SetImageForContext(1, imageManager.GetPaletteManager()->GetPaletteImage()->GetRHITexture());

	drawUIBuffer.modulationColor[0] = command.taskRotateSprite.spriteColor.x;
	drawUIBuffer.modulationColor[1] = command.taskRotateSprite.spriteColor.y;
	drawUIBuffer.modulationColor[2] = command.taskRotateSprite.spriteColor.z;
	
	if (!command.taskRotateSprite.useOrtho)
	{
		memcpy(drawVSUIBuffer.projectionMatrix, projectionMatrixBuild, sizeof(float) * 16);
	}
	else
	{
		memcpy(drawVSUIBuffer.projectionMatrix, projectionMatrixOrtho, sizeof(float) * 16);
	}

	drawUIConstantBuffer->UpdateBuffer(&drawUIBuffer, sizeof(PS_DRAWUI_BUFFER), 0);
	drawVSUIConstantBuffer->UpdateBuffer(&drawVSUIBuffer, sizeof(VS_DRAWUI_BUFFER), 0);
	rhi.SetConstantBuffer(0, drawVSUIConstantBuffer, SHADER_BIND_VERTEXSHADER);
	rhi.SetConstantBuffer(0, drawUIConstantBuffer, SHADER_BIND_PIXELSHADER);
	rhi.DrawUnoptimized2DQuad(corrected_vertexes);
}