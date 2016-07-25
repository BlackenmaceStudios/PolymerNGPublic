// Renderer.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

Renderer renderer;

int currentGameSubmitTime = 0;
int startTimeForGameFrame = 0;
int gameExecTimeInMilliseconds = 0;
int gpuExecTimeInMilliseconds = 0;

int GetCurrentTimeInMilliseconds()
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	return st.wMilliseconds;
}

Renderer::Renderer()
{
	currentFrame = 0;
	memset(&numRenderCommands, 0, sizeof(int) * MAX_SMP_FRAMES);
	num2DRenderCommands = 0;
	currentRenderCommand = NULL;
	currentNumRenderCommand = 0;
	vlsShadowLightMap = NULL;
	vlsLight = NULL;
}

void Renderer::Init()
{
	// Load in all of our shaders.
	ui_texture_basic						= PolymerNGRenderProgram::LoadRenderProgram("guishader", true);
	ui_texture_hq_basic						= PolymerNGRenderProgram::LoadRenderProgram("guishaderHighQuality", true);
	albedoSimpleProgram						= PolymerNGRenderProgram::LoadRenderProgram("AlbedoSimple", false);
	albedoSimpleTransparentProgram			= PolymerNGRenderProgram::LoadRenderProgram("AlbedoSimpleTransparent", false);
	albedoSimpleGlowProgram					= PolymerNGRenderProgram::LoadRenderProgram("AlbedoSimpleGlow", false);
	AlbedoSimpleGlowMapProgram				= PolymerNGRenderProgram::LoadRenderProgram("AlbedoSimpleGlowMap", false);
	albedoHQProgram							= PolymerNGRenderProgram::LoadRenderProgram("AlbedoHighQuality", false);
	albedoHQGlowProgram						= PolymerNGRenderProgram::LoadRenderProgram("AlbedoHighQualityGlow", false);
	albedoHQTransparentProgram				= PolymerNGRenderProgram::LoadRenderProgram("AlbedoHighQualityTransparent", false);
	albedoHQNoNormalMapGlowProgram			= PolymerNGRenderProgram::LoadRenderProgram("AlbedoHighQualityNoNormalMapGlow", false);
	albedoHQNoNormalMapProgram				= PolymerNGRenderProgram::LoadRenderProgram("AlbedoHighQualityNoNormalMap", false);
	spriteSimpleProgram						= PolymerNGRenderProgram::LoadRenderProgram("SpriteSimple", false);
	spriteSimpleGlowProgram					= PolymerNGRenderProgram::LoadRenderProgram("SpriteSimpleGlow", false);
	spriteSimpleGlowMapProgram				= PolymerNGRenderProgram::LoadRenderProgram("SpriteSimpleGlowMap", false); 
	spriteHQProgram							= PolymerNGRenderProgram::LoadRenderProgram("SpriteHighQuality", false);
	spriteHQGlowProgram						= PolymerNGRenderProgram::LoadRenderProgram("SpriteHighQualityGlow", false);
	spriteSimpleHorizProgram				= PolymerNGRenderProgram::LoadRenderProgram("SpriteSimpleHoriz", false);
	postProcessProgram						= PolymerNGRenderProgram::LoadRenderProgram("PostProcess", false);
	classicFSProgram						= PolymerNGRenderProgram::LoadRenderProgram("ClassicScreenFS", false);
	bokehDOFProgram							= PolymerNGRenderProgram::LoadRenderProgram("BokehDOF", false);
	volumetricLightScatterProgram			= PolymerNGRenderProgram::LoadRenderProgram("VolumetricLightScatter", false);

	// Point light render programs
	deferredLightingPointLightProgram			= PolymerNGRenderProgram::LoadRenderProgram("DeferredLightingPointLight", false);
	deferredLightingPointLightNoShadowsProgram	= PolymerNGRenderProgram::LoadRenderProgram("DeferredLightingPointLightNoShadow", false);

	// Spot light render program
	deferredLightingSpotLightProgram			= PolymerNGRenderProgram::LoadRenderProgram("DeferredLightingSpotLight", false);
	deferredLightingSpotLightNoShadowsProgram	= PolymerNGRenderProgram::LoadRenderProgram("DeferredLightingSpotLightNoShadow", false);

	// Load in the AA programs.
	fxaaProgram = PolymerNGRenderProgram::LoadRenderProgram("FXAA", false);

	// Initilizes the different draw passes.
	drawUIPass.Init();
	drawWorldPass.Init();
	drawSpritePass.Init();
	drawClassicSkyPass.Init();
	drawLightingPass.Init();
	drawPostProcessPass.Init();
	classicFSPass.Init();
	dofPass.Init();
	drawAAPass.Init();
	vlsPass.Init();

	// Initilize the shadows
	InitShadowMaps();

	gpuPerfCounter = BuildRHI::AllocatePerformanceCounter();
}

void Renderer::SetShaderForPSO(BuildRHIPipelineStateObject *pso, PolymerNGRenderProgram *program)
{

}

void Renderer::SubmitFrame(SceneNextPageParms nextpageParams)
{
	currentGameSubmitTime = GetCurrentTimeInMilliseconds();

	gameExecTimeInMilliseconds = currentGameSubmitTime - startTimeForGameFrame;

	while (HasWork())
		Sleep(0);

	currentNextPageParam = nextpageParams;
	currentRenderCommand = commands[currentFrame];
	currentFrame = !currentFrame;
	currentNumRenderCommand = numRenderCommands[!currentFrame];
	numRenderCommands[!currentFrame] = 0;

	startTimeForGameFrame = GetCurrentTimeInMilliseconds();
}

bool Renderer::HasWork()
{
	return currentRenderCommand != NULL && currentNumRenderCommand > 0; // numRenderCommands[currentFrame] != 0;
}

void Renderer::RenderFrame()
{
	bool shouldClear = true;
	polymerNG.UploadPendingImages();

	std::vector<BuildRenderCommand> lightDrawCommands;
	BuildRenderCommand *drawWorldCommand = NULL;
	if (gpuPerfCounter)
	{
		//gpuPerfCounter->Begin();
	}

	// Ensure all images loaded(we need a precache system, this is temporary).
	for (int i = 0; i < currentNumRenderCommand; i++)
	{		
		BuildRenderCommand &command = currentRenderCommand[i];

		if (command.taskId == BUILDRENDER_TASK_DRAWCLASSICSCREEN)
		{
			classicFSPass.Draw(command);
		}
		else if (command.taskId == BUILDRENDER_TASK_ROTATESPRITE && command.taskRotateSprite.is2D)
		{
			// For some reason we are either getting bashed memory, or somehting wierd is going on, either way check the texnum value so we don't crash.
			if (command.taskRotateSprite.texnum > MAXTILES)
			{
				initprintf("RendererDrawPassDrawUI::Draw: We have are trying to draw a tile that is out of bounds!\n");
				return;
			}

			if (!GetNextPageParms().shouldSkipUI)
			{
				_2dcommands[num2DRenderCommands++] = &command;
			}
		}
		else if (command.taskId == BUILDRENDER_TASK_CREATEMODEL)
		{
			BaseModel *model = command.taskCreateModel.model;
			if (model->rhiVertexBufferStatic == NULL)
			{
				BuildRHIMesh *rhiMeshData = rhi.AllocateRHIMesh(sizeof(Build3DVertex), model->meshVertexes.size(), &model->meshVertexes[0], true);
				if (model->meshIndexes.size() > 0)
				{
					rhi.AllocateRHIMeshIndexes(rhiMeshData, model->meshIndexes.size(), &model->meshIndexes[0], false);
				}
				model->rhiVertexBufferStatic = rhiMeshData;
			}

			// Create the dynamic buffers.
			//if (command.taskCreateModel.createDynamicBuffers)
			//{
			//	for (int d = 0; d < 2; d++)
			//	{
			//		BuildRHIMesh *rhiMeshData = rhi.AllocateRHIMesh(sizeof(Build3DVertex), command.taskCreateModel.numVertexes, &model->meshVertexes[0], true);
			//		if (model->meshIndexes.size() > 0)
			//		{
			//			rhi.SetRHIMeshIndexBuffer(rhiMeshData, model->rhiVertexBufferStatic);
			//		}
			//		model->rhiVertexBufferDynamic[d] = rhiMeshData;
			//	}
			//}
		}
		else if (command.taskId == BUILDRENDER_TASK_UPDATEMODEL)
		{
			BuildRHIMesh *model = command.taskUpdateModel.rhiMesh;

			if (!model)
				continue;
			int32_t numUpdateItems = command.taskUpdateModel.model->numUpdateQueues;
			ModelUpdateQueuedItem *queuedItems = &command.taskUpdateModel.model->modelUpdateQueue[0];
			for (int d = 0; d < numUpdateItems; d++)
			{
				rhi.UpdateRHIMesh(model, queuedItems[d].startPosition, sizeof(Build3DVertex), queuedItems[d].numVertexes, &command.taskUpdateModel.model->meshVertexes[queuedItems[d].startPosition]);
			}
			command.taskUpdateModel.model->numUpdateQueues = 0;
		}
		else if (command.taskId == BUILDRENDER_TASK_RENDERWORLD)
		{
			// Draw the visibility logic.
			//drawVisSectors.Draw(command);

			// Normal albedo pass.
			if (shouldClear)
			{
				rhi.SetDepthEnable(false);
				drawClassicSkyPass.Draw(command);
				rhi.SetDepthEnable(true);
			}
			else
			{
				rhi.SetDepthEnable(true);
			}

			drawWorldPass.BindDrawWorldRenderTarget(true, shouldClear);
			rhi.ToggleDeferredRenderContext(true);
			drawWorldPass.Draw(command);
			drawWorldCommand = &command;
			rhi.ToggleDeferredRenderContext(false);

			
		}
		else if (command.taskId == BUILDRENDER_TASK_DRAWSPRITES)
		{
			if (!GetNextPageParms().shouldSkipSprites)
			{
				drawSpritePass.Draw(command);
			}

			if (drawWorldCommand)
			{
				drawWorldPass.DrawTrans(*drawWorldCommand);
				drawWorldCommand = NULL;
			}
			drawWorldPass.BindDrawWorldRenderTarget(false, false);
		}
		else if (command.taskId == BUILDRENDER_TASK_DRAWLIGHTS)
		{
			lightDrawCommands.push_back(command);
			shouldClear = false;
		}
	}

	for (int i = 0; i < lightDrawCommands.size(); i++)
	{
		drawLightingPass.shouldClear = (i == 0);
		drawLightingPass.Draw(lightDrawCommands[i]);
	}
	
	drawPostProcessPass.Draw(currentRenderCommand[currentNumRenderCommand - 1]);
	if (!renderer.GetNextPageParms().shouldSkipDOFAndAA)
	{
		dofPass.Draw(currentRenderCommand[currentNumRenderCommand - 1]);
		drawAAPass.Draw(currentRenderCommand[currentNumRenderCommand - 1]);
	}
	

	if (gpuPerfCounter)
	{
		//gpuPerfCounter->End();

		//gpuExecTimeInMilliseconds = gpuPerfCounter->GetTime();
	}
}


void Renderer::RenderFrame2D(class GraphicsContext& Context)
{
	for (int i = 0; i < num2DRenderCommands; i++)
	{
		drawUIPass.Draw( *_2dcommands[i]);
	}

	num2DRenderCommands = 0;
	currentNumRenderCommand = 0;
	currentRenderCommand = NULL;
}