// Renderer_Pass_VolumetricLightScatter.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
=====================
RendererDrawPassVolumetricLightScatter::Init
=====================
*/
void RendererDrawPassVolumetricLightScatter::Init()
{
	BuildImage *diffuseRenderBuffer;
	{
		BuildImageOpts opts;
		opts.width = globalWindowWidth / 6;
		opts.height = globalWindowHeight / 6;
		opts.format = IMAGE_FORMAT_RGBA8;
		opts.isHighQualityImage = true;
		opts.isRenderTargetImage = true;

		diffuseRenderBuffer = new BuildImage(opts);
		diffuseRenderBuffer->UpdateImagePost(NULL);
	}
	renderTarget = new PolymerNGRenderTarget(diffuseRenderBuffer, NULL, NULL);
	vlsConstantBufferPixel = rhi.AllocateRHIConstantBuffer(sizeof(PS_DRAWVLS_BUFFER), &psDrawVLSBuffer);
}

/*
=====================
RendererDrawPassVolumetricLightScatter::Draw
=====================
*/
void RendererDrawPassVolumetricLightScatter::Draw(const BuildRenderCommand &command)
{
	if (renderer.vlsLight == NULL || renderer.vlsShadowLightMap == NULL)
		return;

	psDrawVLSBuffer.LightPosAndRadius.x = renderer.vlsLight->GetOpts()->position[0];
	psDrawVLSBuffer.LightPosAndRadius.y = renderer.vlsLight->GetOpts()->position[1];
	psDrawVLSBuffer.LightPosAndRadius.z = renderer.vlsLight->GetOpts()->position[2];
	psDrawVLSBuffer.LightPosAndRadius.w = renderer.vlsLight->GetOpts()->radius;

	psDrawVLSBuffer.LightColor.x = ((float)renderer.vlsLight->GetOpts()->color[0]) / 255.0f;
	psDrawVLSBuffer.LightColor.y = ((float)renderer.vlsLight->GetOpts()->color[1]) / 255.0f;
	psDrawVLSBuffer.LightColor.z = ((float)renderer.vlsLight->GetOpts()->color[2]) / 255.0f;

	//psDrawVLSBuffer.CameraPos

	vlsConstantBufferPixel->UpdateBuffer(&psDrawVLSBuffer, sizeof(PS_DRAWVLS_BUFFER), 0);
	rhi.SetConstantBuffer(0, vlsConstantBufferPixel, SHADER_BIND_PIXELSHADER);

	renderTarget->Bind();
	rhi.SetShader(renderer.volumetricLightScatterProgram->GetRHIShader());
	rhi.SetImageForContext(0, renderer.GetWorldDepthBuffer()->GetRHITexture());
	rhi.SetImageForContext(1, renderer.vlsShadowLightMap->shadowMapCubeMap->GetDepthImage()->GetRHITexture());
	rhi.DrawUnoptimized2DQuad(NULL);
	PolymerNGRenderTarget::BindDeviceBuffer();
}