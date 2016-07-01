// PolymerNG_RenderTarget.cpp
//

#include "PolymerNG_local.h"

PolymerNGRenderTarget::PolymerNGRenderTarget(BuildImage *diffuseImage, BuildImage *depthImage, BuildImage *stencilImage)
{
	this->diffuseImage[0] = diffuseImage;
	this->depthImage = depthImage;
	this->stencilImage = stencilImage;

	BuildRHITexture *diffuseRHIImage = NULL;
	BuildRHITexture *depthRHIImage = NULL;
	BuildRHITexture *stencilRHIImage = NULL;

	if (diffuseImage)
	{
		diffuseRHIImage = (BuildRHITexture*)diffuseImage->GetRHITexture();
	}

	if (depthImage)
	{
		depthRHIImage = (BuildRHITexture*)depthImage->GetRHITexture();
	}

	if (stencilImage)
	{
		stencilRHIImage = (BuildRHITexture*)stencilImage->GetRHITexture();
	}

	numRenderTargets = 1;
	renderTarget = BuildRHI::AllocateRHIRenderTarget(diffuseRHIImage, depthRHIImage, stencilRHIImage);
}

void PolymerNGRenderTarget::AddRenderTaret(BuildImage *diffuseImage)
{
	this->diffuseImage[numRenderTargets++] = diffuseImage;
	renderTarget->AddRenderTarget((BuildRHITexture*)diffuseImage->GetRHITexture());
}

void PolymerNGRenderTarget::Bind(int slice, bool shouldClear)
{
	BuildRHI::BindRenderTarget(renderTarget, slice, shouldClear);
}

void PolymerNGRenderTarget::BindDeviceBuffer()
{
	BuildRHI::BindRenderTarget(NULL, 0, false);
}