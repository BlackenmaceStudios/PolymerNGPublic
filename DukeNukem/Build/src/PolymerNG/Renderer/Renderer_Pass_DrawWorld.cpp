// Renderer_Pass_DrawWorld.cpp
//

#include "Renderer.h"
#include "build3d.h"
#include "../PolymerNG_local.h"
#include <mutex>

/*
========================
RendererDrawPassDrawWorld::Init
========================
*/
void RendererDrawPassDrawWorld::Init()
{
	drawWorldConstantBuffer = rhi.AllocateRHIConstantBuffer(sizeof(VS_DRAWWORLD_BUFFER), &drawWorldBuffer);
}

/*
========================
RendererDrawPassDrawWorld::DrawPlane
========================
*/
void RendererDrawPassDrawWorld::DrawPlane(BuildRHIMesh *rhiMesh, const BaseModel *model, const Build3DPlane *plane)
{
	BuildImage *image = static_cast<BuildImage *>(plane->renderImageHandle);

	if (image != NULL)
	{
		rhi.SetImageForContext(0, image->GetRHITexture());
		rhi.SetImageForContext(1, polymerNG.GetPaletteImage()->GetRHITexture());

		if (plane->ibo_offset != -1)
		{
			rhi.DrawIndexedQuad(renderer.albedoSimpleProgram->GetRHIShader(), rhiMesh, plane->vbo_offset, plane->ibo_offset, plane->indicescount);
		}
		else
		{
			rhi.DrawUnoptimizedQuad(renderer.albedoSimpleProgram->GetRHIShader(), rhiMesh, plane->vbo_offset, plane->vertcount);
		}
	}
}

/*
========================
RendererDrawPassDrawWorld::Draw
========================
*/
void RendererDrawPassDrawWorld::Draw(const BuildRenderCommand &command)
{
	drawWorldBuffer.mWorldViewProj = command.taskRenderWorld.viewProjMatrix;
	drawWorldConstantBuffer->UpdateBuffer(&drawWorldBuffer, sizeof(VS_DRAWWORLD_BUFFER), 0);

	rhi.SetConstantBuffer(0, drawWorldConstantBuffer);

	const Build3DBoard *board = command.taskRenderWorld.board;

	int currentSMPFrame = renderer.GetCurrentFrameNum();

	// Render all of the static sectors.
	for (int i = 0; i < command.taskRenderWorld.numRenderPlanes; i++)
	{
		if (!command.taskRenderWorld.renderplanes[i]->isDynamicPlane)
		{
			DrawPlane(board->GetBaseModel()->rhiVertexBufferStatic, board->GetBaseModel(), command.taskRenderWorld.renderplanes[i]);
		}
		else
		{
			DrawPlane(board->GetBaseModel()->rhiVertexBufferDynamic[currentSMPFrame], board->GetBaseModel(), command.taskRenderWorld.renderplanes[i]);
		}
	}

//	((Build3DBoard *)board)->GetBaseModel()->geoUpdateQueue[command.taskRenderWorld.gameSmpFrame].clear();

	//for (int i = 0; i < numsectors; i++)
	//{
	//	// Draw the sectors.
	//	const Build3DSector *sector = board->GetSector(i);
	//	tsectortype *mapSector = (tsectortype *)&::sector[i];
	//	
	//	if (!sector->IsCeilParalaxed())
	//	{
	//		DrawPlane(board->GetBaseModel(), &sector->ceil);
	//	}
	//
	//	if (!sector->IsFloorParalaxed())
	//	{
	//		DrawPlane(board->GetBaseModel(), &sector->floor);
	//	}
	//
	//	// Draw all the walls for this sector.
	//	for (int d = 0; d < mapSector->wallnum; d++)
	//	{
	//		const Build3DWall *wall = board->GetWall(mapSector->wallptr + d);
	//		const Build3DSector *neighborSector = NULL;
	//	
	//		/*
	//		==============================================
	//		
	//		Hidden walls
	//	
	//		The logic here is if there is a parralax sector, wrapped inside of another paralax sector, don't draw the walls.
	//	
	//		==============================================
	//		*/
	//		
	//		if (::wall[mapSector->wallptr + d].nextsector >= 0)
	//		{
	//			neighborSector = board->GetSector(::wall[mapSector->wallptr + d].nextsector);
	//		}
	//	
	//		if (wallvisible(globalposx, globalposy, mapSector->wallptr + d))
	//		{
	//			if ((wall->underover & 1) && neighborSector == NULL || neighborSector && !sector->IsFloorParalaxed() && !neighborSector->IsFloorParalaxed())
	//			{
	//				DrawPlane(board->GetBaseModel(), &wall->wall);
	//			}
	//	
	//			if ((wall->underover & 2) && neighborSector == NULL || neighborSector && !sector->IsCeilParalaxed() && !neighborSector->IsCeilParalaxed())
	//			{
	//				DrawPlane(board->GetBaseModel(), &wall->over);
	//			}
	//		}
	//	}
	//}


}