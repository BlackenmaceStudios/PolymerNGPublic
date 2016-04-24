// Renderer.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

Renderer renderer;

static bool hasRenderWork = false;

Renderer::Renderer()
{
	currentFrame = 0;
	

}

void Renderer::Init()
{
	// Load in all of our shaders.
	ui_texture_basic = PolymerNGRenderProgram::LoadRenderProgram("guishader");
	albedoSimpleProgram = PolymerNGRenderProgram::LoadRenderProgram("AlbedoSimple");
	spriteSimpleProgram = PolymerNGRenderProgram::LoadRenderProgram("SpriteSimple");
	spriteSimpleHorizProgram = PolymerNGRenderProgram::LoadRenderProgram("SpriteSimpleHoriz");

	// Initilizes the different draw passes.
	drawUIPass.Init();
	drawWorldPass.Init();
	drawSpritePass.Init();
	drawClassicSkyPass.Init();
}

void Renderer::SetShaderForPSO(BuildRHIPipelineStateObject *pso, PolymerNGRenderProgram *program)
{

}

void Renderer::SubmitFrame()
{
	currentFrame = !currentFrame;
	while (commands[!currentFrame].size() != 0)
		Sleep(0);

	hasRenderWork = true;
}

bool Renderer::HasWork()
{
	return commands[!currentFrame].size() != 0;
}

void Renderer::RenderFrame()
{
	polymerNG.UploadPendingImages();

	std::vector<BuildRenderCommand>	&frame_commands = commands[!currentFrame];
	

	// Ensure all images loaded(we need a precache system, this is temporary).
	for (int i = 0; i < frame_commands.size(); i++)
	{
		BuildRenderCommand &command = frame_commands[i];

		if (command.taskId == BUILDRENDER_TASK_ROTATESPRITE && command.taskRotateSprite.is2D)
		{
			// For some reason we are either getting bashed memory, or somehting wierd is going on, either way check the texnum value so we don't crash.
			if (command.taskRotateSprite.texnum > MAXTILES)
			{
				initprintf("RendererDrawPassDrawUI::Draw: We have are trying to draw a tile that is out of bounds!\n");
				return;
			}

			BuildImage *image = polymerNG.GetImage(command.taskRotateSprite.texnum);

			if (!image->IsLoaded())
				image->UpdateImagePost(NULL);

			_2dcommands.push_back(command);
		}
		else if (command.taskId == BUILDRENDER_TASK_CREATEMODEL)
		{
			BaseModel *model = command.taskCreateModel.model;
			if (model->rhiVertexBufferStatic == NULL)
			{
				BuildRHIMesh *rhiMeshData = rhi.AllocateRHIMesh(sizeof(Build3DVertex), command.taskCreateModel.numVertexes, &model->meshVertexes[0], false);
				if (model->meshIndexes.size() > 0)
				{
					rhi.AllocateRHIMeshIndexes(rhiMeshData, model->meshIndexes.size(), &model->meshIndexes[0], false);
				}
				model->rhiVertexBufferStatic = rhiMeshData;
			}

			// Create the dynamic buffers.
			if (command.taskCreateModel.createDynamicBuffers)
			{
				for (int d = 0; d < 2; d++)
				{
					BuildRHIMesh *rhiMeshData = rhi.AllocateRHIMesh(sizeof(Build3DVertex), command.taskCreateModel.numVertexes, &model->meshVertexes[0], true);
					if (model->meshIndexes.size() > 0)
					{
						rhi.SetRHIMeshIndexBuffer(rhiMeshData, model->rhiVertexBufferStatic);
					}
					model->rhiVertexBufferDynamic[d] = rhiMeshData;
				}
			}
		}
		else if (command.taskId == BUILDRENDER_TASK_UPDATEMODEL)
		{
			BuildRHIMesh *model = command.taskUpdateModel.rhiMesh;

			int32_t numUpdateItems = command.taskUpdateModel.modelUpdateQueuedItems.size();
			ModelUpdateQueuedItem *queuedItems = &command.taskUpdateModel.modelUpdateQueuedItems[0];
			//for (int d = 0; d < numUpdateItems; d++)
			//{
			//	rhi.UpdateRHIMesh(model, queuedItems[d].startPosition, sizeof(Build3DVertex), queuedItems[d].numVertexes, &command.taskUpdateModel.model->meshVertexes[queuedItems[d].startPosition]);
			//}
			rhi.UpdateRHIMesh(model, 0, sizeof(Build3DVertex), command.taskUpdateModel.model->meshVertexes.size(), &command.taskUpdateModel.model->meshVertexes[0]);

			command.taskUpdateModel.modelUpdateQueuedItems.clear();
		}
		else if (command.taskId == BUILDRENDER_TASK_RENDERWORLD)
		{
			rhi.SetDepthEnable(false);
			drawClassicSkyPass.Draw(command);
			rhi.SetDepthEnable(true);
			drawWorldPass.Draw(command);
		}
		else if (command.taskId == BUILDRENDER_TASK_DRAWSPRITES)
		{
			drawSpritePass.Draw(command);
		}
	}
	
	frame_commands.clear();
}

void Renderer::RenderFrame2D(class GraphicsContext& Context)
{
	for (int i = 0; i < _2dcommands.size(); i++)
	{
		drawUIPass.Draw( _2dcommands[i]);
	}
	_2dcommands.clear();
	hasRenderWork = false;
}