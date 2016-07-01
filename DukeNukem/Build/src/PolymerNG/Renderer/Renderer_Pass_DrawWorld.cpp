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
	BuildImage *normalRenderBuffer;
	BuildImage *normalMapRenderBuffer;
	BuildImage *tangentRenderBuffer;
	BuildImage *depthRenderBuffer;
	BuildImage *specularGlowPropertyRenderBuffer;

	drawWorldConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_DRAWWORLD_BUFFER), &drawWorldBuffer);
	drawWorldPixelConstantBuffer[0] = rhi.AllocateRHIConstantBuffer(sizeof(PS_CONSTANT_BUFFER), &drawWorldPixelBuffer);
	drawWorldPixelConstantBuffer[1] = rhi.AllocateRHIConstantBuffer(sizeof(PS_CONSTANT_BUFFER), &drawWorldPixelBuffer);

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_RGBA8;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		diffuseRenderBuffer = new BuildImage(opts);
		diffuseRenderBuffer->UpdateImagePost(NULL);
	}

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_R10G10B10A2;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		specularGlowPropertyRenderBuffer = new BuildImage(opts);
		specularGlowPropertyRenderBuffer->UpdateImagePost(NULL);
	}

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_RGBA8;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		normalMapRenderBuffer = new BuildImage(opts);
		normalMapRenderBuffer->UpdateImagePost(NULL);
	}

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_RGB16;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		tangentRenderBuffer = new BuildImage(opts);
		tangentRenderBuffer->UpdateImagePost(NULL);
	}

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_RGB16;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		normalRenderBuffer = new BuildImage(opts);
		normalRenderBuffer->UpdateImagePost(NULL);
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

	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth;
		opts.height = globalWindowHeight;
		opts.format = IMAGE_FORMAT_DEPTH;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = false;

		depthRenderBuffer_copied = new BuildImage(opts);
		depthRenderBuffer_copied->UpdateImagePost(NULL);
	}

	renderTarget = new PolymerNGRenderTarget(diffuseRenderBuffer, depthRenderBuffer, NULL);
	renderTarget->AddRenderTaret(normalRenderBuffer);
	renderTarget->AddRenderTaret(normalMapRenderBuffer);
	renderTarget->AddRenderTaret(tangentRenderBuffer);
	renderTarget->AddRenderTaret(specularGlowPropertyRenderBuffer);
}

/*
========================
RendererDrawPassDrawWorld::DrawPlane
========================
*/
void RendererDrawPassDrawWorld::DrawPlane(BuildRHIMesh *rhiMesh, const BaseModel *model, const Build3DPlane *plane)
{
	static bool useDoubleBuffer = false;
	PolymerNGMaterial *material = static_cast<PolymerNGMaterial *>(plane->renderMaterialHandle);

	if (material != NULL)
	{
		drawWorldPixelBuffer.shadeOffsetVisibility[0] = plane->shadeNum;
		drawWorldPixelBuffer.shadeOffsetVisibility[1] = plane->visibility;
		drawWorldPixelBuffer.fogColor[0] = plane->fogColor[0];
		drawWorldPixelBuffer.fogColor[1] = plane->fogColor[1];
		drawWorldPixelBuffer.fogColor[2] = plane->fogColor[2];

		drawWorldPixelBuffer.fogDensistyScaleEnd[0] = plane->fogDensity;
		drawWorldPixelBuffer.fogDensistyScaleEnd[1] = plane->fogStart;
		drawWorldPixelBuffer.fogDensistyScaleEnd[2] = plane->fogEnd;

		drawWorldPixelBuffer.tangent[0] = plane->tbn[0][0];
		drawWorldPixelBuffer.tangent[1] = plane->tbn[1][0];
		drawWorldPixelBuffer.tangent[2] = plane->tbn[2][0];

		drawWorldPixelBuffer.normal[0] = plane->tbn[0][2];
		drawWorldPixelBuffer.normal[1] = plane->tbn[1][2];
		drawWorldPixelBuffer.normal[2] = plane->tbn[2][2];


		drawWorldPixelConstantBuffer[useDoubleBuffer]->UpdateBuffer(&drawWorldPixelBuffer, sizeof(PS_CONSTANT_BUFFER), 0);
		rhi.SetConstantBuffer(0, drawWorldPixelConstantBuffer[useDoubleBuffer], SHADER_BIND_PIXELSHADER);

		rhi.SetImageForContext(0, material->GetDiffuseTexture()->GetRHITexture());
		rhi.SetImageForContext(1, imageManager.GetPaletteManager()->GetPaletteImage()->GetRHITexture());
		rhi.SetImageForContext(2, imageManager.GetPaletteManager()->GetPaletteLookupImage(plane->paletteNum)->GetRHITexture());		

		BuildRHIShader *shader = renderer.albedoSimpleProgram->GetRHIShader();
		if (material->GetDiffuseTexture()->GetOpts().isHighQualityImage)
		{
			if (material->GetNormalMap() == NULL)
			{
				shader = renderer.albedoHQNoNormalMapProgram->GetRHIShader();
			}
			else
			{
				rhi.SetImageForContext(3, material->GetNormalMap()->GetRHITexture());

				if (material->GetSpecularMap())
				{
					rhi.SetImageForContext(4, material->GetSpecularMap()->GetRHITexture());
				}
				else
				{
					rhi.SetImageForContext(4, imageManager.GetBlackImage()->GetRHITexture());
				}
				shader = renderer.albedoHQProgram->GetRHIShader();
			}
		}

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
	if (enable)
	{
		renderTarget->Bind();
	}
	else
	{
		rhi.CopyDepthToAnotherImage(renderTarget->GetDepthImage()->GetRHITexture(), depthRenderBuffer_copied->GetRHITexture());
		PolymerNGRenderTarget::BindDeviceBuffer();
	}
}

/*
========================
RendererDrawPassDrawWorld::Draw
========================
*/
void RendererDrawPassDrawWorld::Draw(const BuildRenderCommand &command)
{
	rhi.SetFaceCulling(CULL_FACE_FRONT);

	drawWorldBuffer.viewPosition[0] = -command.taskRenderWorld.position.GetX();
	drawWorldBuffer.viewPosition[1] = -command.taskRenderWorld.position.GetY();
	drawWorldBuffer.viewPosition[2] = -command.taskRenderWorld.position.GetZ();
	drawWorldBuffer.viewPosition[3] = 1.0f; // command.taskRenderWorld.position.GetW();
	drawWorldBuffer.mWorldViewProj = command.taskRenderWorld.viewProjMatrix;
	drawWorldBuffer.mProjectionMatrix = command.taskRenderWorld.projectionMatrix;
	drawWorldBuffer.mWorldView = command.taskRenderWorld.viewMatrix;
	drawWorldBuffer.inverseViewMatrix = command.taskRenderWorld.inverseViewMatrix;
	drawWorldConstantBuffer->UpdateBuffer(&drawWorldBuffer, sizeof(VS_DRAWWORLD_BUFFER), 0);

	rhi.SetConstantBuffer(0, drawWorldConstantBuffer, SHADER_BIND_VERTEXSHADER);

	const Build3DBoard *board = command.taskRenderWorld.board;

	int currentSMPFrame = renderer.GetCurrentFrameNum();

//	drawWorldPixelConstantBuffer->UpdateBuffer(&drawWorldPixelBuffer, sizeof(PS_CONSTANT_BUFFER), 0);
//	rhi.SetConstantBuffer(0, drawWorldPixelConstantBuffer, SHADER_BIND_PIXELSHADER);

	// Render all of the static sectors.
	for (int i = 0; i < command.taskRenderWorld.numRenderPlanes; i++)
	{
		DrawPlane(board->GetBaseModel()->rhiVertexBufferStatic, board->GetBaseModel(), command.taskRenderWorld.renderplanes[i]);
	}
}