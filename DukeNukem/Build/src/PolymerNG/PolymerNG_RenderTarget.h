// PolymerNG_RenderTarget.h
//

#pragma once

//
// PolymerNGRenderTarget
// 
class PolymerNGRenderTarget
{
public:
	PolymerNGRenderTarget(BuildImage *diffuseImage, BuildImage *depthImage, BuildImage *stencilImage);

	void AddRenderTaret(BuildImage *diffuseImage);

	void Bind(int slice = 0, bool shouldClear = true);

	BuildImage *GetDiffuseImage(int idx) { return diffuseImage[idx]; }
	BuildImage *GetDepthImage() { return depthImage; }
	BuildImage *GetStencilImage() { return stencilImage; }

	static void BindDeviceBuffer();
private:
	BuildImage *diffuseImage[MAX_RENDER_TARGETS];
	BuildImage *depthImage;
	BuildImage *stencilImage;
	BuildRHIRenderTarget *renderTarget;
	int numRenderTargets;
};
