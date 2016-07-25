// Renderer_Pass_Lighting.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
=====================
RendererDrawPassLighting::Init
=====================
*/
void RendererDrawPassLighting::Init()
{
	BuildImage *diffuseRenderBuffer, *depthRenderBuffer;
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
	renderTarget = new PolymerNGRenderTarget(diffuseRenderBuffer, NULL, NULL);
	drawLightingConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(PS_DRAWLIGHTING_BUFFER), &drawLightingBuffer);
}

/*
=====================
RendererDrawPassLighting::Draw
=====================
*/
void RendererDrawPassLighting::Draw(const BuildRenderCommand &command)
{
	PolymerNGRenderTarget *drawWorldRenderTarget = renderer.GetWorldRenderTarget();
	ShadowMap *shadowMapPool[NUM_QUEUED_SHADOW_MAPS];
	int numShadowedLights = 0;

	renderer.vlsLight = NULL;
	renderer.vlsShadowLightMap = NULL;

	rhi.SetBlendState(BLENDSTATE_ALPHA);
	rhi.SetFaceCulling(CULL_FACE_BACK);
	
	for (int i = 0; i < command.taskDrawLights.numLights; i++)
	{
		if (command.taskDrawLights.visibleLights[i]->GetOpts()->castShadows)
		{
			shadowMapPool[numShadowedLights] = renderer.RenderShadowsForLight(command.taskDrawLights.visibleLights[i], numShadowedLights);
			numShadowedLights++;
		}

		if (command.taskDrawLights.visibleLights[i]->GetOpts()->enableVolumetricLight)
		{
			renderer.vlsShadowLightMap = shadowMapPool[numShadowedLights - 1];
			renderer.vlsLight = command.taskDrawLights.visibleLights[i];
		}
	}
	rhi.SetFaceCulling(CULL_FACE_NONE);
	rhi.SetBlendState(BLENDSTATE_ADDITIVE);

	renderTarget->Bind(0, shouldClear);

	for (int i = 0, d  = 0; i < command.taskDrawLights.numLights; i++)
	{
		ShadowMap *shadowMap = NULL;
		
		if (command.taskDrawLights.visibleLights[i]->GetOpts()->castShadows)
		{
			shadowMap = shadowMapPool[d++];
		}
		
		if (command.taskDrawLights.visibleLights[i]->GetOpts()->castShadows)
		{
			switch (command.taskDrawLights.visibleLights[i]->GetOpts()->lightType)
			{
				case POLYMERNG_LIGHTTYPE_POINT:
					rhi.SetShader(renderer.deferredLightingPointLightProgram->GetRHIShader());
					break;
				case POLYMERNG_LIGHTTYPE_SPOT:
					rhi.SetShader(renderer.deferredLightingSpotLightProgram->GetRHIShader());
					break;
			}
		}
		else
		{
			switch (command.taskDrawLights.visibleLights[i]->GetOpts()->lightType)
			{
			case POLYMERNG_LIGHTTYPE_POINT:
				rhi.SetShader(renderer.deferredLightingPointLightNoShadowsProgram->GetRHIShader());
				break;
			case POLYMERNG_LIGHTTYPE_SPOT:
				rhi.SetShader(renderer.deferredLightingSpotLightNoShadowsProgram->GetRHIShader());
				break;
			}
		}

		rhi.SetImageForContext(0, drawWorldRenderTarget->GetDiffuseImage(0)->GetRHITexture());
		rhi.SetImageForContext(1, drawWorldRenderTarget->GetDiffuseImage(1)->GetRHITexture());
		rhi.SetImageForContext(2, drawWorldRenderTarget->GetDiffuseImage(2)->GetRHITexture());
		rhi.SetImageForContext(3, drawWorldRenderTarget->GetDiffuseImage(3)->GetRHITexture());
		rhi.SetImageForContext(4, drawWorldRenderTarget->GetDiffuseImage(4)->GetRHITexture());
		rhi.SetImageForContext(5, renderer.GetWorldDepthBuffer()->GetRHITexture());

		if (command.taskDrawLights.visibleLights[i]->GetOpts()->castShadows)
		{
			switch (command.taskDrawLights.visibleLights[i]->GetOpts()->lightType)
			{
				case POLYMERNG_LIGHTTYPE_POINT:
					rhi.SetImageForContext(6, shadowMap->shadowMapCubeMap->GetDepthImage()->GetRHITexture(), true);
					break;
				case POLYMERNG_LIGHTTYPE_SPOT:
					rhi.SetImageForContext(6, shadowMap->spotLightMap->GetDepthImage()->GetRHITexture(), true);
					break;
			}
			
		}

		drawLightingBuffer.cameraposition = command.taskDrawLights.cameraposition;

		drawLightingBuffer.spotDir = float4(command.taskDrawLights.visibleLights[i]->GetShadowPass(0)->spotdir, 1.0);
		drawLightingBuffer.spotRadius = float4(command.taskDrawLights.visibleLights[i]->GetShadowPass(0)->spotRadius, 1.0);

		drawLightingBuffer.viewMatrix = command.taskDrawLights.viewMatrix;
		drawLightingBuffer.invModelViewProjectionMatrix = command.taskDrawLights.inverseModelViewMatrix;
		drawLightingBuffer.inverseViewMatrix = command.taskDrawLights.inverseViewMatrix;

		drawLightingBuffer.lightViewMatrix = command.taskDrawLights.visibleLights[i]->GetShadowPass(0)->shadowViewMatrix;
		drawLightingBuffer.lightProjectionMatrix = command.taskDrawLights.visibleLights[i]->GetShadowPass(0)->shadowProjectionMatrix;

		drawLightingBuffer.lightposition_and_range[0] = command.taskDrawLights.visibleLights[i]->GetOpts()->position[1];
		drawLightingBuffer.lightposition_and_range[1] = -command.taskDrawLights.visibleLights[i]->GetOpts()->position[2] / 16.0f;
		drawLightingBuffer.lightposition_and_range[2] = -command.taskDrawLights.visibleLights[i]->GetOpts()->position[0];
		drawLightingBuffer.lightposition_and_range[3] = command.taskDrawLights.visibleLights[i]->GetOpts()->radius;

		drawLightingBuffer.lightrangcolor[0] = command.taskDrawLights.visibleLights[i]->GetOpts()->color[0] / 255.0f;
		drawLightingBuffer.lightrangcolor[1] = command.taskDrawLights.visibleLights[i]->GetOpts()->color[1] / 255.0f;
		drawLightingBuffer.lightrangcolor[2] = command.taskDrawLights.visibleLights[i]->GetOpts()->color[2] / 255.0f;
		drawLightingBuffer.lightrangcolor[3] = command.taskDrawLights.visibleLights[i]->GetOpts()->radius;

		drawLightingBuffer.lightbrightnessandunknown.x = command.taskDrawLights.visibleLights[i]->GetOpts()->brightness;

		drawLightingBuffer.numLightsUnkown[0] = 1;

		drawLightingConstantBuffer->UpdateBuffer(&drawLightingBuffer, sizeof(PS_DRAWLIGHTING_BUFFER), 0);
		rhi.SetConstantBuffer(0, drawLightingConstantBuffer, SHADER_BIND_PIXELSHADER);
		rhi.DrawUnoptimized2DQuad(NULL);
	}
	rhi.SetBlendState(BLENDSTATE_ALPHA);
	

	

	PolymerNGRenderTarget::BindDeviceBuffer();
}