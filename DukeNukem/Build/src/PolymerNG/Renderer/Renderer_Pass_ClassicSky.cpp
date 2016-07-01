// Renderer_Draw_ClassicSky.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

const int numClassicSkyPlanes = 8;

/*
=====================
RendererDrawPassDrawClassicSky::Init
=====================
*/
void RendererDrawPassDrawClassicSky::Init()
{
	float         halfsqrt2 = 0.70710678f;

	artskydata[0] = -1.0f;          artskydata[1] = 0.0f;           // 0
	artskydata[2] = -halfsqrt2;     artskydata[3] = halfsqrt2;      // 1
	artskydata[4] = 0.0f;           artskydata[5] = 1.0f;           // 2
	artskydata[6] = halfsqrt2;      artskydata[7] = halfsqrt2;      // 3
	artskydata[8] = 1.0f;           artskydata[9] = 0.0f;           // 4
	artskydata[10] = halfsqrt2;     artskydata[11] = -halfsqrt2;    // 5
	artskydata[12] = 0.0f;          artskydata[13] = -1.0f;         // 6
	artskydata[14] = -halfsqrt2;    artskydata[15] = -halfsqrt2;    // 7

	classicSkyRHIMesh = rhi.AllocateRHIMesh(sizeof(Build3DVertex), numClassicSkyPlanes * 4, NULL, true);

	classicSkyConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_DRAWCLASSICSKY_BUFFER), &drawClassicSkyBuffer);
}

/*
=====================
RendererDrawPassDrawClassicSky::Draw
=====================
*/
void RendererDrawPassDrawClassicSky::Draw(const BuildRenderCommand &command)
{
	Build3DVertex classicSkyVertexes[8 * 4];
	float         height = 2.45f / 2.0f;

	for (int i = 0; i < numClassicSkyPlanes; i++)
	{
		int32_t p1 = i;
		int32_t p2 = (i + 1) & (numClassicSkyPlanes - 1);

		classicSkyVertexes[(i * 4) + 1].SetVertex(Build3DVector4(artskydata[(p1 * 2) + 1], height, artskydata[p1 * 2], 1.0f), Build3DVector4(0.0f, 0.0f, 0.0f, 0.0f));
		classicSkyVertexes[(i * 4) + 0].SetVertex(Build3DVector4(artskydata[(p1 * 2) + 1], -height, artskydata[p1 * 2], 1.0f), Build3DVector4(0.0f, 1.0f, 0.0f, 0.0f));
		classicSkyVertexes[(i * 4) + 2].SetVertex(Build3DVector4(artskydata[(p2 * 2) + 1], -height, artskydata[p2 * 2], 1.0f), Build3DVector4(1.0f, 1.0f, 0.0f, 0.0f));
		classicSkyVertexes[(i * 4) + 3].SetVertex(Build3DVector4(artskydata[(p2 * 2) + 1], height, artskydata[p2 * 2], 1.0f), Build3DVector4(1.0f, 0.0f, 0.0f, 0.0f));
	}

	rhi.UpdateRHIMesh(classicSkyRHIMesh, 0, sizeof(Build3DVertex), numClassicSkyPlanes * 4, classicSkyVertexes);

	PolymerNGMaterial *material = static_cast<PolymerNGMaterial *>(command.taskRenderWorld.skyMaterialHandle);
	if (material == NULL)
		return;

	drawClassicSkyBuffer.mWorldViewProj = command.taskRenderWorld.skyProjMatrix;
	//drawSpriteBuffer.modelMatrix = sprite->modelMatrix;
	classicSkyConstantBuffer->UpdateBuffer(&drawClassicSkyBuffer, sizeof(VS_DRAWCLASSICSKY_BUFFER), 0);

	rhi.SetConstantBuffer(0, classicSkyConstantBuffer, SHADER_BIND_VERTEXSHADER);

	rhi.SetImageForContext(0, material->GetDiffuseTexture()->GetRHITexture());
	rhi.SetImageForContext(1, imageManager.GetPaletteManager()->GetPaletteImage()->GetRHITexture());

	rhi.DrawUnoptimizedQuad(renderer.albedoSimpleProgram->GetRHIShader(), classicSkyRHIMesh, 0, numClassicSkyPlanes * 4);
}