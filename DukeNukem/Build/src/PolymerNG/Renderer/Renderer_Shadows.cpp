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

	drawShadowPointLightConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_SHADOW_POINT_CONSTANT_BUFFER), &drawShadowPointLightBuffer);

	pointLightShadowFaceMatrix[0] = CreateLookAtMatrix(float3::ZeroF, float3::Right, float3::Up); // +X
	pointLightShadowFaceMatrix[1] = CreateLookAtMatrix(float3::ZeroF, float3::Left,  float3::Up); // -X
	pointLightShadowFaceMatrix[2] = CreateLookAtMatrix(float3::ZeroF, float3::Up, float3::Backward); // +Y
	pointLightShadowFaceMatrix[3] = CreateLookAtMatrix(float3::ZeroF, float3::Down, float3::Forward); // -Y
	pointLightShadowFaceMatrix[4] = CreateLookAtMatrix(float3::ZeroF, float3::Forward, float3::Up); // +Z
	pointLightShadowFaceMatrix[5] = CreateLookAtMatrix(float3::ZeroF, float3::Backward, float3::Up); // -Z


	for (int i = 0; i < NUM_QUEUED_SHADOW_MAPS; i++)
	{
		BuildImage *depthRenderBuffer, *colorRenderBuffer;

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

	if (light->GetOpts()->lightType == POLYMERNG_LIGHTTYPE_POINT)
	{
		BuildRHIShader *shader = cubemapShadowMapProgram->GetRHIShader();
		rhi.SetShader(shader);

		for (int i = 0; i < 6; i++)
		{
			shadowMap->shadowMapCubeMap->Bind(i);

			const PolymerNGShadowPass *lightShadowPass = light->GetShadowPass(i);

			//float3 lightPosition(opts.position[1], -opts.position[2] / 16.0f, -opts.position[0]);
			float4 lightposition(light->GetOpts()->position[1], -light->GetOpts()->position[2] / 16.0f, -light->GetOpts()->position[0], 0.0f);

			drawShadowPointLightBuffer.mWorldView = lightShadowPass->shadowViewProjectionMatrix;
			drawShadowPointLightBuffer.light_position_and_range = lightposition;
			drawShadowPointLightBuffer.light_position_and_range.w = light->GetOpts()->radius;
			drawShadowPointLightConstantBuffer->UpdateBuffer(&drawShadowPointLightBuffer, sizeof(VS_SHADOW_POINT_CONSTANT_BUFFER), 0);
			rhi.SetConstantBuffer(0, drawShadowPointLightConstantBuffer, SHADER_BIND_VERTEXSHADER);

			for (int d = 0; d < light->GetShadowPass(i)->shadowOccluders[shadowOccluderFrame].size(); d++)
			{
				const Build3DPlane *plane = lightShadowPass->shadowOccluders[shadowOccluderFrame][d].plane;
				if (plane->ibo_offset != -1)
				{
					rhi.DrawIndexedQuad(shader, light->GetRHIMesh(), 0, plane->ibo_offset, plane->indicescount);
				}
				else
				{
					rhi.DrawUnoptimizedQuad(shader, light->GetRHIMesh(), plane->vbo_offset, plane->vertcount);
				}
			}
		}
	}
	else
	{
		initprintf("Renderer::RenderShadowsForLight: Shadow Mapping for %d not implemented yet!", light->GetOpts()->lightType);
	}
	

	return shadowMap;
}