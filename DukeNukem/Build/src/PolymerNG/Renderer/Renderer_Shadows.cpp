// Renderer.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
==================
Renderer::InitShadowMaps
==================
*/
void Renderer::InitShadowMaps()
{
	dualParabloidShadowMapProgram = PolymerNGRenderProgram::LoadRenderProgram("ShadowDP", false);
	cubemapShadowMapProgram = PolymerNGRenderProgram::LoadRenderProgram("ShadowCUBE", false);
	cubemapShadowAlphaMapProgram = PolymerNGRenderProgram::LoadRenderProgram("ShadowCUBEWithAlpha", false);
	spotlightShadowMapProgram = PolymerNGRenderProgram::LoadRenderProgram("ShadowSPOT", false);
	spotlightShadowAlphaMapProgram = PolymerNGRenderProgram::LoadRenderProgram("ShadowSPOTWithAlpha", false);

	drawShadowPointLightConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_SHADOW_POINT_CONSTANT_BUFFER), &drawShadowPointLightBuffer);

	pointLightShadowFaceMatrix[0] = CreateLookAtMatrix(float3::ZeroF, float3::Right, float3::Up); // +X
	pointLightShadowFaceMatrix[1] = CreateLookAtMatrix(float3::ZeroF, float3::Left,  float3::Up); // -X
	pointLightShadowFaceMatrix[2] = CreateLookAtMatrix(float3::ZeroF, float3::Up, float3::Backward); // +Y
	pointLightShadowFaceMatrix[3] = CreateLookAtMatrix(float3::ZeroF, float3::Down, float3::Forward); // -Y
	pointLightShadowFaceMatrix[4] = CreateLookAtMatrix(float3::ZeroF, float3::Forward, float3::Up); // +Z
	pointLightShadowFaceMatrix[5] = CreateLookAtMatrix(float3::ZeroF, float3::Backward, float3::Up); // -Z


	for (int i = 0; i < NUM_QUEUED_SHADOW_MAPS; i++)
	{
		BuildImage *depthRenderBuffer, *depthRenderBuffer2D;

		{
			BuildImageOpts opts;
			opts.width = SHADOW_MAP_SIZE;
			opts.height = SHADOW_MAP_SIZE;
			opts.format = IMAGE_FORMAT_DEPTH;
			opts.imageType = IMAGETYPE_CUBE;
			opts.isHighQualityImage = true;
			opts.isRenderTargetImage = false;

			depthRenderBuffer = new BuildImage(opts);
			depthRenderBuffer->UpdateImagePost(NULL);
		}

		{
			BuildImageOpts opts;
			opts.width = SHADOW_MAP_SIZE;
			opts.height = SHADOW_MAP_SIZE;
			opts.format = IMAGE_FORMAT_DEPTH;
			opts.imageType = IMAGETYPE_2D;
			opts.isHighQualityImage = true;
			opts.isRenderTargetImage = false;

			depthRenderBuffer2D = new BuildImage(opts);
			depthRenderBuffer2D->UpdateImagePost(NULL);
		}

//		{
//			BuildImageOpts opts;
//			opts.width = SHADOW_MAP_SIZE;
//			opts.height = SHADOW_MAP_SIZE;
//			opts.format = IMAGE_FORMAT_R16_FLOAT;
//			opts.imageType = IMAGETYPE_CUBE;
//			opts.isHighQualityImage = true;
//			opts.isRenderTargetImage = true;
//
//			colorRenderBuffer = new BuildImage(opts);
//			colorRenderBuffer->UpdateImagePost(NULL);
//		}

		shadowMaps[i].shadowMapCubeMap = new PolymerNGRenderTarget(NULL, depthRenderBuffer, NULL);
		shadowMaps[i].spotLightMap = new PolymerNGRenderTarget(NULL, depthRenderBuffer2D, NULL);
	}
}

/*
==================
Renderer::RenderShadowsForLight
==================
*/
ShadowMap *Renderer::RenderShadowsForLight(PolymerNGLightLocal *light, int shadowId)
{
	int shadowOccluderFrame = renderer.GetCurrentFrameNum();
	ShadowMap *shadowMap = &this->shadowMaps[shadowId];

	int numShadowMapPasses = 6;

	if (light->GetOpts()->lightType == POLYMERNG_LIGHTTYPE_SPOT)
	{
		numShadowMapPasses = 1;
	}
	else if (light->GetOpts()->lightType == POLYMERNG_LIGHTTYPE_POINT)
	{
		numShadowMapPasses = 6;
	}
	else
	{
		initprintf("Renderer::RenderShadowsForLight: Shadow Mapping for %d not implemented yet!", light->GetOpts()->lightType);
		return NULL;
	}

	for (int i = 0; i < numShadowMapPasses; i++)
	{
		if (light->GetOpts()->lightType == POLYMERNG_LIGHTTYPE_SPOT)
		{
			shadowMap->spotLightMap->Bind(0, true);
		}
		else if (light->GetOpts()->lightType == POLYMERNG_LIGHTTYPE_POINT)
		{
			shadowMap->shadowMapCubeMap->Bind(i);
		}

		const PolymerNGShadowPass *lightShadowPass = light->GetShadowPass(i);

		//float3 lightPosition(opts.position[1], -opts.position[2] / 16.0f, -opts.position[0]);
		float4 lightposition(light->GetOpts()->position[1], -light->GetOpts()->position[2] / 16.0f, -light->GetOpts()->position[0], 0.0f);

		drawShadowPointLightBuffer.mWorldView = lightShadowPass->shadowViewProjectionMatrix;
		drawShadowPointLightBuffer.light_position_and_range = lightposition;
		drawShadowPointLightBuffer.light_position_and_range.w = light->GetOpts()->radius;
		drawShadowPointLightBuffer.mModelMatrix = float4x4Identity();
		drawShadowPointLightConstantBuffer->UpdateBuffer(&drawShadowPointLightBuffer, sizeof(VS_SHADOW_POINT_CONSTANT_BUFFER), 0);
		rhi.SetConstantBuffer(0, drawShadowPointLightConstantBuffer, SHADER_BIND_VERTEXSHADER);

		// Render the sectors first.
		for (int d = 0; d < light->GetShadowPass(i)->shadowOccluders[shadowOccluderFrame].size(); d++)
		{
			if (!light->GetShadowPass(i)->shadowOccluders[shadowOccluderFrame][d].plane)
				continue;

			const Build3DPlane *plane = lightShadowPass->shadowOccluders[shadowOccluderFrame][d].plane;
			PolymerNGMaterial *material = static_cast<PolymerNGMaterial *>(plane->renderMaterialHandle);

			BuildRHIShader *shader = NULL; 
			if (light->GetOpts()->lightType == POLYMERNG_LIGHTTYPE_SPOT)
			{
				if (plane->isMaskWall || material->GetGlowMap() != NULL)
				{
					shader = spotlightShadowAlphaMapProgram->GetRHIShader();
				}
				else
				{
					shader = spotlightShadowMapProgram->GetRHIShader();
				}
			}
			else if (light->GetOpts()->lightType == POLYMERNG_LIGHTTYPE_POINT)
			{
				if (plane->isMaskWall || (material && material->GetGlowMap() != NULL))
				{
					shader = cubemapShadowAlphaMapProgram->GetRHIShader();
				}
				else
				{
					shader = cubemapShadowMapProgram->GetRHIShader();
				}

			}

			if ((material && material->GetGlowMap() && material->GetGlowMap()->GetRHITexture() != NULL))
			{
				rhi.SetFaceCulling(CULL_FACE_NONE);
				rhi.SetImageForContext(0, material->GetGlowMap()->GetRHITexture(), true);
			}
			else if (plane->isMaskWall)
			{
				rhi.SetFaceCulling(CULL_FACE_NONE);
				rhi.SetImageForContext(0, material->GetDiffuseTexture()->GetRHITexture(), true);
			}
			else
			{
				rhi.SetFaceCulling(CULL_FACE_FRONT);
			}

			if (plane->ibo_offset != -1)
			{
				rhi.DrawIndexedQuad(shader, light->GetRHIMesh(), 0, plane->ibo_offset, plane->indicescount);
			}
			else
			{
				rhi.DrawUnoptimizedQuad(shader, light->GetRHIMesh(), plane->vbo_offset, plane->vertcount);
			}
		}

		rhi.SetFaceCulling(CULL_FACE_NONE);

		// Render the sprites next
		for (int d = 0; d < light->GetShadowPass(i)->shadowOccluders[shadowOccluderFrame].size(); d++)
		{
			const Build3DSprite *sprite = &light->GetShadowPass(i)->shadowOccluders[shadowOccluderFrame][d].sprite;

			if (!sprite->cacheModel)
				continue;

			drawShadowPointLightBuffer.mModelMatrix = sprite->modelMatrix;
			drawShadowPointLightBuffer.mWorldView = sprite->modelViewProjectionMatrix;
			drawShadowPointLightBuffer.light_position_and_range = lightposition;
			drawShadowPointLightBuffer.light_position_and_range.w = light->GetOpts()->radius;
			drawShadowPointLightConstantBuffer->UpdateBuffer(&drawShadowPointLightBuffer, sizeof(VS_SHADOW_POINT_CONSTANT_BUFFER), 0);
			rhi.SetConstantBuffer(0, drawShadowPointLightConstantBuffer, SHADER_BIND_VERTEXSHADER);

			for (int f = 0; f < sprite->cacheModel->GetNumSurfaces(); f++)
			{
				CacheModelSurface *surface = sprite->cacheModel->GetCacheSurface(f);

				if (sprite->cacheModel->GetBaseModel() == NULL)
					continue;

				BuildRHIShader *shader = NULL;
				if (light->GetOpts()->lightType == POLYMERNG_LIGHTTYPE_POINT)
				{
					shader = cubemapShadowMapProgram->GetRHIShader();
				}
				rhi.DrawIndexedQuad(shader, sprite->cacheModel->GetBaseModel()->rhiVertexBufferStatic, 0, surface->startIndex, surface->numIndexes);
			}
		}
	}

	return shadowMap;
}