// Renderer.h
//

#pragma once

#include "compat.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <vector>

class BuildRHIPipelineStateObject;

extern float gtang;
extern float fxdim, fydim, fydimen, fviewingrange;
extern float fxdimen;
extern int32_t globalposx, globalposy, globalposz, globalhoriz;

//
// RendererDrawPassBase
//
class RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init() = 0;

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command) = 0;
protected:
	BuildRHIPipelineStateObject *pso;
};

#include "Renderer_Pass_DrawUI.h"
#include "Renderer_Pass_DrawWorld.h"
#include "Renderer_Pass_DrawSprite.h"

#define MAX_SMP_FRAMES		2

//
// Renderer
//
class Renderer
{
public:
	Renderer();

	void		Init();

	void		SetShaderForPSO(BuildRHIPipelineStateObject *pso, PolymerNGRenderProgram *program);

	void		SubmitFrame();

	void		RenderFrame();

	bool		HasWork();

	void		RenderFrame2D(class GraphicsContext& Context);

	void		AddRenderCommand(BuildRenderCommand &command) { commands[currentFrame].push_back(command); }

	PolymerNGRenderProgram *ui_texture_basic;
	PolymerNGRenderProgram *albedoSimpleProgram;
	PolymerNGRenderProgram *spriteSimpleProgram;
	PolymerNGRenderProgram *spriteSimpleHorizProgram;
private:
	std::vector<BuildRenderCommand>	commands[MAX_SMP_FRAMES];

	std::vector<BuildRenderCommand> _2dcommands;

	RendererDrawPassDrawUI drawUIPass;
	RendererDrawPassDrawWorld drawWorldPass;
	RendererDrawPassDrawSprite drawSpritePass;
	int			currentFrame;
};

extern Renderer renderer;
