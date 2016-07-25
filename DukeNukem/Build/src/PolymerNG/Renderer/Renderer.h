// Renderer.h
//

#pragma once

#include "compat.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <vector>

class BuildRHIPipelineStateObject;

#include "../../engine_priv.h"

extern float fydimen;
extern float gtang;
extern float globalWindowWidth;
extern float globalWindowHeight;

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
#include "Renderer_Pass_Lighting.h"
#include "Renderer_Pass_PostProcess.h"
#include "Renderer_Pass_ClassicFS.h"
#include "Renderer_Pass_DOF.h"
#include "Renderer_Pass_AntiAlias.h"
#include "Renderer_Pass_VolumetricLightScatter.h"

#include "Renderer_Shadows.h"

#define MAX_SMP_FRAMES		2
#define MAX_RENDER_COMMANDS 800

#define VISPASS_WIDTH		274
#define VISPASS_HEIGHT		154

//
// Renderer
//
class Renderer
{
public:
	Renderer();

	void		Init();

	void		SetShaderForPSO(BuildRHIPipelineStateObject *pso, PolymerNGRenderProgram *program);

	void		SubmitFrame(SceneNextPageParms nextpageParams);

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

	BuildImage *GetPreviousFrameImage() { return drawWorldPass.GetPreviousRenderFrame(); }

	PolymerNGRenderTarget  *GetWorldRenderTarget() { return drawWorldPass.GetDrawWorldRenderTarget(); }
	PolymerNGRenderTarget  *GetHDRLightingRenderTarget() { return drawLightingPass.GetHDRLightingBuffer(); }
	PolymerNGRenderTarget  *GetPostProcessRenderTarget() { return drawPostProcessPass.GetPostProcessRenderTarget(); }
	PolymerNGRenderTarget  *GetDOFRenderTarget() { return dofPass.GetDOFRenderTarget(); }

	PolymerNGRenderProgram *ui_texture_basic;
	PolymerNGRenderProgram *ui_texture_hq_basic;
	PolymerNGRenderProgram *albedoSimpleProgram;
	PolymerNGRenderProgram *albedoSimpleTransparentProgram;
	PolymerNGRenderProgram *albedoSimpleGlowProgram;
	PolymerNGRenderProgram *AlbedoSimpleGlowMapProgram;
	PolymerNGRenderProgram *albedoHQProgram;
	PolymerNGRenderProgram *albedoHQTransparentProgram;
	PolymerNGRenderProgram *albedoHQGlowProgram;
	PolymerNGRenderProgram *albedoHQNoNormalMapProgram;
	PolymerNGRenderProgram *albedoHQNoNormalMapGlowProgram;
	PolymerNGRenderProgram *spriteSimpleProgram;
	PolymerNGRenderProgram *spriteSimpleGlowProgram;
	PolymerNGRenderProgram *spriteSimpleGlowMapProgram;
	PolymerNGRenderProgram *spriteHQProgram;
	PolymerNGRenderProgram *spriteHQGlowProgram;
	PolymerNGRenderProgram *spriteSimpleHorizProgram;

	PolymerNGRenderProgram *deferredLightingPointLightProgram;
	PolymerNGRenderProgram *deferredLightingPointLightNoShadowsProgram;
	PolymerNGRenderProgram *deferredLightingSpotLightProgram;
	PolymerNGRenderProgram *deferredLightingSpotLightNoShadowsProgram;
	PolymerNGRenderProgram *postProcessProgram;
	PolymerNGRenderProgram *classicFSProgram;
	PolymerNGRenderProgram *fxaaProgram;
	PolymerNGRenderProgram *volumetricLightScatterProgram;

	PolymerNGRenderProgram *bokehDOFProgram;

// Shadow code Begin
public:
	ShadowMap *RenderShadowsForLight(PolymerNGLightLocal *light, int shadowId);

	BuildImage *GetWorldDepthBuffer() { return drawWorldPass.GetWorldDepthImage(); }

	SceneNextPageParms GetNextPageParms() { return currentNextPageParam; }

	float4x4 pointLightShadowFaceMatrix[6];
public:
	ShadowMap *vlsShadowLightMap;
	PolymerNGLightLocal *vlsLight;
private:
	void InitShadowMaps();

	PolymerNGRenderProgram *dualParabloidShadowMapProgram;
	PolymerNGRenderProgram *cubemapShadowMapProgram;
	PolymerNGRenderProgram *cubemapShadowAlphaMapProgram;
	PolymerNGRenderProgram *spotlightShadowMapProgram;
	PolymerNGRenderProgram *spotlightShadowAlphaMapProgram;
	VS_SHADOW_POINT_CONSTANT_BUFFER drawShadowPointLightBuffer;
	BuildRHIConstantBuffer		*drawShadowPointLightConstantBuffer;
private:
	int numRenderCommands[MAX_SMP_FRAMES];
	BuildRenderCommand	commands[MAX_SMP_FRAMES][MAX_RENDER_COMMANDS];
	SceneNextPageParms  currentNextPageParam;

	int num2DRenderCommands;
	BuildRenderCommand *_2dcommands[MAX_RENDER_COMMANDS];

	RendererDrawPassAA drawAAPass;
	RendererDrawPassDrawUI drawUIPass;
	RendererDrawPassDrawWorld drawWorldPass;
	RendererDrawPassDrawSprite drawSpritePass;
	RendererDrawPassDrawClassicSky drawClassicSkyPass;
	RendererDrawPassLighting drawLightingPass;
	RendererDrawPassPostProcess drawPostProcessPass;
	RendererDrawPassDrawClassicScreenFS classicFSPass;
	RendererDrawPassDOF dofPass;
	RendererDrawPassVolumetricLightScatter vlsPass;

	BuildRHIGPUPerformanceCounter *gpuPerfCounter;
	
	int			currentFrame;

	BuildRenderCommand *currentRenderCommand;
	int currentNumRenderCommand;

	ShadowMap	shadowMaps[NUM_QUEUED_SHADOW_MAPS];
};

extern Renderer renderer;
