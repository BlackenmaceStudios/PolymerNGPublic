// Renderer_Pass_ClassicFS.h
//

#pragma once

class RendererDrawPassDrawClassicScreenFS : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:
	BuildImage					*classScreenImageFS;
	byte						*gpuScreenBuffer;
};

