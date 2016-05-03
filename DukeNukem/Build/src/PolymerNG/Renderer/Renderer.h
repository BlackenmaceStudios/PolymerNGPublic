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
#include "Renderer_Pass_ClassicSky.h"

#define MAX_SMP_FRAMES		2
#define MAX_RENDER_COMMANDS 400
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

	void		AddRenderCommand(BuildRenderCommand &command) {
		if (numRenderCommands[currentFrame] >= MAX_RENDER_COMMANDS)
		{ 
			initprintf("AddRenderCommand: MAX_RENDER_COMMANDS exceeded!!!...\n"); 
			return;
		} 
		commands[currentFrame][numRenderCommands[currentFrame]++] = command;
	}

	int			GetCurrentFrameNum() { return currentFrame; }

	PolymerNGRenderProgram *ui_texture_basic;
	PolymerNGRenderProgram *albedoSimpleProgram;
	PolymerNGRenderProgram *spriteSimpleProgram;
	PolymerNGRenderProgram *spriteSimpleHorizProgram;
private:
	int numRenderCommands[MAX_SMP_FRAMES];
	BuildRenderCommand	commands[MAX_SMP_FRAMES][MAX_RENDER_COMMANDS];

	int num2DRenderCommands;
	BuildRenderCommand *_2dcommands[MAX_RENDER_COMMANDS];

	RendererDrawPassDrawUI drawUIPass;
	RendererDrawPassDrawWorld drawWorldPass;
	RendererDrawPassDrawSprite drawSpritePass;
	RendererDrawPassDrawClassicSky drawClassicSkyPass;
	int			currentFrame;

	BuildRenderCommand *currentRenderCommand;
	int currentNumRenderCommand;
};

extern Renderer renderer;
