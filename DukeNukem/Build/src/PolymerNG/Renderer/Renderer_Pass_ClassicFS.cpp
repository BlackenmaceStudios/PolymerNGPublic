// Renderer_Pass_ClassicFS.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
=====================
RendererDrawPassDrawClassicScreenFS::Init
=====================
*/
void RendererDrawPassDrawClassicScreenFS::Init()
{
	BuildImageOpts opts;
	opts.width = globalWindowWidth;
	opts.height = globalWindowHeight;
	opts.format = IMAGE_FORMAT_RGBA8;
	opts.isHighQualityImage = true;
	opts.isRenderTargetImage = false;
	opts.heapType = BUILDIMAGE_ALLOW_CPUWRITES;

	classScreenImageFS = new BuildImage(opts);
	classScreenImageFS->UpdateImagePost(NULL);

	gpuScreenBuffer = new byte[globalWindowWidth * globalWindowHeight * 4];
}

/*
=====================
RendererDrawPassDrawClassicScreenFS::Draw
=====================
*/
void RendererDrawPassDrawClassicScreenFS::Draw(const BuildRenderCommand &command)
{
	for (int i = 0, d = 0; i < globalWindowWidth * globalWindowHeight * 4; i += 4, d++)
	{
		palette_t pal = curpalette[command.taskDrawClassicScreen.screen_buffer[d]];
		gpuScreenBuffer[i + 0] = pal.r;
		gpuScreenBuffer[i + 1] = pal.g;
		gpuScreenBuffer[i + 2] = pal.b;
		gpuScreenBuffer[i + 3] = 255;
	}

	classScreenImageFS->UpdateImagePost(gpuScreenBuffer);
	rhi.SetShader(renderer.classicFSProgram->GetRHIShader());
	rhi.SetImageForContext(0, classScreenImageFS->GetRHITexture());
	rhi.DrawUnoptimized2DQuad(NULL);
}