// Renderer_Pass_DOF.h
//

#pragma once

class RendererDrawPassDOF : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);

	PolymerNGRenderTarget		*GetDOFRenderTarget() { return renderTarget; }
private:
	PolymerNGRenderTarget		*renderTarget;
};
