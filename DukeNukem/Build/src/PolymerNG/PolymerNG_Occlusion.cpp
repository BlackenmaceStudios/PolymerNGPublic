// PolymerNG_Occlusion.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

//#define DISABLE_CULLING 1

enum PolymerNGOcclusionTest
{
	POLYMER_OCCLUSION_NOTEST = 0,
	POLYMER_OCCLUSION_TEST
};


/*
=============
PolymerNGBoard::InitOcclusion
=============
*/
void PolymerNGBoard::InitOcclusion()
{
	initprintf("Initializing occlusion engine for map\n");
}

/*
=============
PolymerNGBoard::RenderOccluderFromPlane
=============
*/
void PolymerNGBoard::RenderOccluderFromPlane(const Math::Matrix4 &modelViewProjectionMatrix, const Build3DPlane *plane)
{

}

/*
=============
PolymerNGBoard::IsSectorVisible
=============
*/
bool PolymerNGBoard::IsSectorVisible(const Math::Matrix4 &modelViewProjectionMatrix, Build3DSector *sector)
{
	return true;
}

/*
=============
PolymerNGBoard::AddRenderPlaneToDrawList
=============
*/
void PolymerNGBoard::AddRenderPlaneToDrawList(BuildRenderThreadTaskRenderWorld &renderWorldTask, Build3DPlane *plane)
{
	Build3D::CalculateFogForPlane(plane->tileNum, plane->shadeNum, plane->visibility, plane->paletteNum, plane);
	renderWorldTask.renderplanes[renderWorldTask.numRenderPlanes++] = plane;
}

/*
=============
PolymerNGBoard::ScanSprites
=============
*/
void PolymerNGBoard::ScanSprites(int16_t sectnum, tspritetype* localtsprite, int32_t* localspritesortcnt)
{
	int32_t         i;
	spritetype      *spr;

	for (i = headspritesect[sectnum]; i >= 0; i = nextspritesect[i])
	{
		spr = &sprite[i];

		if ((((spr->cstat & 0x8000) == 0) || (showinvisibility)) &&
			(spr->xrepeat > 0) && (spr->yrepeat > 0) &&
			(*localspritesortcnt < MAXSPRITESONSCREEN))
		{
			// this function's localtsprite is either the tsprite global or
			// polymer_drawroom's locattsprite, so no aliasing
			Bmemcpy(&localtsprite[*localspritesortcnt], spr, sizeof(spritetype));
			localtsprite[(*localspritesortcnt)++].owner = i;
		}
	}
}
/*
=============
PolymerNGBoard::PokeSector
=============
*/
void PolymerNGBoard::PokeSector(int16_t sectnum)
{
	sectortype      *sec = &sector[sectnum];
	Build3DSector   *s = board->GetSector(sectnum);
	walltype        *wal = &wall[sec->wallptr];
	int32_t         i = 0;

	if (!s->flags.uptodate)
		board->updatesector(sectnum);

	do
	{
		if ((wal->nextsector >= 0) && (!board->GetSector(wal->nextsector)->flags.uptodate))
			board->updatesector(wal->nextsector);
		if (!board->GetWall(sec->wallptr + i)->flags.uptodate)
			board->updatewall(sec->wallptr + i);

		i++;
		wal = &wall[sec->wallptr + i];
	} while (i < sec->wallnum);
}

/*
=============
PolymerNGBoard::FindVisibleSectors
=============
*/
void PolymerNGBoard::FindVisibleSectors(BuildRenderThreadTaskRenderWorld &renderWorldTask, const Math::Matrix4 &modelViewProjectionMatrix, int16_t dacursectnum)
{
	int32_t         front;
	int32_t         back;
	int16_t         ns;
	int16_t         bunchnum;
	int16_t			sectorqueue[MAXSECTORS];
	int16_t			drawingstate[MAXSECTORS];
	PolymerNGOcclusionTest		queueStatus[MAXWALLS];
	int16_t         doquery;
	int32_t         i;
	tsectortype      *sec;
	int16_t         localmaskwall[MAXWALLSB];
	int16_t         localmaskwallcnt;

	Bmemset(queueStatus, 0, sizeof(PolymerNGOcclusionTest) * numwalls);
	Bmemset(drawingstate, 0, sizeof(int16_t) * numsectors);
	Bmemset(localtsprite, 0, sizeof(tspritetype) * MAXSPRITESONSCREEN);

	front = 0;
	back = 1;
	sectorqueue[0] = dacursectnum;
	drawingstate[dacursectnum] = 1;

	renderWorldTask.numRenderPlanes = 0;
	localspritesortcnt = localmaskwallcnt = 0;

	//mirrorcount = 0;
	//
	//localsectormasks = (int16_t *)Xmalloc(sizeof(int16_t) * numsectors);
	//localsectormaskcount = (int16_t *)Xcalloc(sizeof(int16_t), 1);
	//cursectormasks = localsectormasks;
	//cursectormaskcount = localsectormaskcount;

	// Unflag all the sectors.
	for (int d = 0; d < numsectors; d++)
	{
		board->GetSector(d)->flags.uptodate = 0;
	}

	for (int d = 0; d < numwalls; d++)
	{
		board->GetWall(d)->flags.uptodate = 0;
	}

	int32_t *visibleSectors = softwareRasterizer->GetVisibleSectorList();
	int numVisibleSectors = softwareRasterizer->GetNumVisibleSectors();

	for (int d = 0; d < numVisibleSectors; d++)
	{
		int sectorNum = visibleSectors[d];

		sec = (tsectortype *)&sector[sectorNum];
		int sectorVisiblity = ((uint8_t)(sec->visibility + 16) / 16.0f);

		// Draw the sectors.
		Build3DSector *sector = board->GetSector(sectorNum);

		PokeSector(sectorNum);

		if (!sector->IsCeilParalaxed() && yax_getbunch(sectorNum, YAX_CEILING) < 0)
		{
			sector->ceil.paletteNum = sec->ceilingpal;
			sector->ceil.shadeNum = sec->ceilingshade;
			sector->ceil.visibility = sectorVisiblity;
			AddRenderPlaneToDrawList(renderWorldTask, &sector->ceil);
		}

		if (!sector->IsFloorParalaxed() && yax_getbunch(sectorNum, YAX_FLOOR) < 0)
		{
			sector->floor.paletteNum = sec->floorpal;
			sector->floor.shadeNum = sec->floorshade;
			sector->floor.visibility = sectorVisiblity;
			AddRenderPlaneToDrawList(renderWorldTask, &sector->floor);
		}

		ScanSprites(sectorNum, localtsprite, &localspritesortcnt);

		i = sec->wallnum - 1;
		do
		{
			int32_t wallNum = sec->wallptr + i;
			if (wallvisible(globalposx, globalposy, wallNum))
			{
				Build3DWall *wall = board->GetWall(wallNum);
				Build3DSector *neighborSector = NULL;
		
				/*
				==============================================
		
				Hidden walls
		
				The logic here is if there is a parralax sector, wrapped inside of another paralax sector, don't draw the walls.
		
				==============================================
				*/
		
				if (::wall[wallNum].nextsector >= 0)
				{
					neighborSector = board->GetSector(::wall[wallNum].nextsector);
				}
		
				bool parralaxFloor = (neighborSector != NULL && neighborSector && sector->IsFloorParalaxed() && neighborSector->IsFloorParalaxed());
				if ((wall->underover & 1) && (!parralaxFloor || searchit == 2))
				{
					wall->wall.paletteNum = ::wall[wallNum].pal;
					wall->wall.shadeNum = ::wall[wallNum].shade;
					wall->wall.visibility = sectorVisiblity;
					AddRenderPlaneToDrawList(renderWorldTask, &wall->wall);
				}
		
				bool parralaxCeiling =  (neighborSector != NULL && neighborSector && sector->IsCeilParalaxed() && neighborSector->IsCeilParalaxed());
				if ((wall->underover & 2) && (!parralaxCeiling || searchit == 2))
				{
					wall->wall.paletteNum = ::wall[wallNum].pal;
					wall->wall.shadeNum = ::wall[wallNum].shade;
					wall->wall.visibility = sectorVisiblity;
					AddRenderPlaneToDrawList(renderWorldTask, &wall->over);
				}
		
				if ((::wall[wallNum].cstat & 32) && (::wall[wallNum].nextsector >= 0))
				{
					wall->wall.paletteNum = ::wall[wallNum].pal;
					wall->wall.shadeNum = ::wall[wallNum].shade;
					wall->wall.visibility = sectorVisiblity;
					AddRenderPlaneToDrawList(renderWorldTask, &wall->mask);
				}
			}
		} while (--i >= 0);
	}

	//	while (front != back)
	//	{
	//		
	//		sec = (tsectortype *)&sector[sectorqueue[front]];
	//		int sectorVisiblity = ((uint8_t)(sec->visibility + 16) / 16.0f);
	//
	//		// Draw the sectors.
	//		Build3DSector *sector = board->GetSector(sectorqueue[front]);
	//
	//		PokeSector(sectorqueue[front]);
	//
	//		if (!sector->IsCeilParalaxed() && yax_getbunch(sectorqueue[front], YAX_CEILING) < 0)
	//		{
	//			sector->ceil.paletteNum = sec->ceilingpal;
	//			sector->ceil.shadeNum = sec->ceilingshade;
	//			sector->ceil.visibility = sectorVisiblity;
	//			AddRenderPlaneToDrawList(renderWorldTask, &sector->ceil);
	//		}
	//
	//		if (!sector->IsFloorParalaxed() && yax_getbunch(sectorqueue[front], YAX_FLOOR) < 0)
	//		{
	//			sector->floor.paletteNum = sec->floorpal;
	//			sector->floor.shadeNum = sec->floorshade;
	//			sector->floor.visibility = sectorVisiblity;
	//			AddRenderPlaneToDrawList(renderWorldTask, &sector->floor);
	//		}
	//
	//		ScanSprites(sectorqueue[front], localtsprite, &localspritesortcnt);
	//
	//		//polymer_pokesector(sectorqueue[front]);
	//		//polymer_drawsector(sectorqueue[front], FALSE);
	//		
	//		doquery = 1;
	//
	//		i = sec->wallnum - 1;
	//		do
	//		{
	//			// if we have a level boundary somewhere in the sector,
	//			// consider these walls as visportals
	//			if (wall[sec->wallptr + i].nextsector < 0)
	//				doquery = 1;
	//		} while (--i >= 0);
	//
	//		i = sec->wallnum - 1;
	//		while (i >= 0)
	//		{
	//			if ((wall[sec->wallptr + i].nextsector >= 0) &&
	//				(wallvisible(globalposx, globalposy, sec->wallptr + i)))
	//			{
	//				if ((board->GetWall(sec->wallptr + i)->mask.vertcount == 4) &&
	//					!(board->GetWall(sec->wallptr + i)->underover & 4) &&
	//					!(board->GetWall(sec->wallptr + i)->underover & 8))
	//				{
	//					// early exit for closed sectors
	//					Build3DWall         *w = board->GetWall(sec->wallptr + i);
	//
	//
	//					if ((w->mask.buffer[0].position.y >= w->mask.buffer[3].position.y) &&
	//						(w->mask.buffer[1].position.y >= w->mask.buffer[2].position.y))
	//					{
	//						i--;
	//						continue;
	//					}
	//				}
	//
	//				if ((wall[sec->wallptr + i].cstat & 48) == 16)
	//				{
	//					int pic = wall[sec->wallptr + i].overpicnum;
	//
	//					//if (tilesiz[pic].x > 0 && tilesiz[pic].y > 0)
	//					//	localmaskwall[localmaskwallcnt++] = sec->wallptr + i;
	//				}
	//
	//				//if (!depth && (overridematerial & prprogrambits[PR_BIT_MIRROR_MAP].bit) &&
	//				//	wall[sec->wallptr + i].overpicnum == 560 &&
	//				//	wall[sec->wallptr + i].cstat & 32)
	//				//{
	//				//	mirrorlist[mirrorcount].plane = &prwalls[sec->wallptr + i]->mask;
	//				//	mirrorlist[mirrorcount].sectnum = sectorqueue[front];
	//				//	mirrorlist[mirrorcount].wallnum = sec->wallptr + i;
	//				//	mirrorcount++;
	//				//}
	//
	//				{
	//					
	//					{
	//						float pos[3], sqdist;
	//						int32_t oldoverridematerial;
	//
	//						pos[0] = (float)globalposy;
	//						pos[1] = -(float)(globalposz) / 16.0f;
	//						pos[2] = -(float)globalposx;
	//
	//						sqdist = board->GetWall(sec->wallptr + i)->mask.plane[0] * pos[0] +
	//								board->GetWall(sec->wallptr + i)->mask.plane[1] * pos[1] +
	//								board->GetWall(sec->wallptr + i)->mask.plane[2] * pos[2] +
	//								board->GetWall(sec->wallptr + i)->mask.plane[3];
	//
	//						// hack to avoid occlusion querying portals that are too close to the viewpoint
	//						// this is needed because of the near z-clipping plane;
	//						//if (sqdist < 100)
	//						//	queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_NOTEST;
	//						//else 
	//						{
	//							queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_TEST;
	//
	//							Build3DWall         *w;
	//
	//							w = board->GetWall(sec->wallptr + i);
	//
	//
	//							if ((w->underover & 4) && (w->underover & 1))
	//							{
	//								RenderOccluderFromPlane(modelViewProjectionMatrix, &w->wall);
	//							}
	//
	//							RenderOccluderFromPlane(modelViewProjectionMatrix, &w->mask);
	//							
	//							if ((w->underover & 8) && (w->underover & 2))
	//							{
	//								RenderOccluderFromPlane(modelViewProjectionMatrix, &w->over);
	//							}
	//						}
	//					}
	//					//else
	//					//	queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_NOTEST;
	//				}
	//			}
	//
	//			i--;
	//		}
	//
	//		// Cram as much CPU or GPU work as we can between queuing the
	//		// occlusion queries and reaping them.
	//		i = sec->wallnum - 1;
	//		do
	//		{
	//			int32_t wallNum = sec->wallptr + i;
	//			if (wallvisible(globalposx, globalposy, wallNum))
	//			{
	//				Build3DWall *wall = board->GetWall(wallNum);
	//				Build3DSector *neighborSector = NULL;
	//
	//				/*
	//				==============================================
	//
	//				Hidden walls
	//
	//				The logic here is if there is a parralax sector, wrapped inside of another paralax sector, don't draw the walls.
	//
	//				==============================================
	//				*/
	//
	//				if (::wall[wallNum].nextsector >= 0)
	//				{
	//					neighborSector = board->GetSector(::wall[wallNum].nextsector);
	//				}
	//
	//				bool parralaxFloor = (neighborSector != NULL && neighborSector && sector->IsFloorParalaxed() && neighborSector->IsFloorParalaxed());
	//				if ((wall->underover & 1) && (!parralaxFloor || searchit == 2))
	//				{
	//					wall->wall.paletteNum = ::wall[wallNum].pal;
	//					wall->wall.shadeNum = ::wall[wallNum].shade;
	//					wall->wall.visibility = sectorVisiblity;
	//					AddRenderPlaneToDrawList(renderWorldTask, &wall->wall);
	//				}
	//
	//				bool parralaxCeiling =  (neighborSector != NULL && neighborSector && sector->IsCeilParalaxed() && neighborSector->IsCeilParalaxed());
	//				if ((wall->underover & 2) && (!parralaxCeiling || searchit == 2))
	//				{
	//					wall->wall.paletteNum = ::wall[wallNum].pal;
	//					wall->wall.shadeNum = ::wall[wallNum].shade;
	//					wall->wall.visibility = sectorVisiblity;
	//					AddRenderPlaneToDrawList(renderWorldTask, &wall->over);
	//				}
	//
	//				if ((::wall[wallNum].cstat & 32) && (::wall[wallNum].nextsector >= 0))
	//				{
	//					wall->wall.paletteNum = ::wall[wallNum].pal;
	//					wall->wall.shadeNum = ::wall[wallNum].shade;
	//					wall->wall.visibility = sectorVisiblity;
	//					AddRenderPlaneToDrawList(renderWorldTask, &wall->mask);
	//				}
	//			}
	//		} while (--i >= 0);
	//#ifdef YAX_ENABLE
	//		// queue ROR neighbors
	//		if ((bunchnum = yax_getbunch(sectorqueue[front], YAX_FLOOR)) >= 0) {
	//
	//			for (SECTORS_OF_BUNCH(bunchnum, YAX_CEILING, ns)) {
	//
	//				if (ns >= 0 && !drawingstate[ns] /*&&
	//					polymer_planeinfrustum(&prsectors[ns]->ceil, frustum)*/) {
	//
	//					sectorqueue[back++] = ns;
	//					drawingstate[ns] = 1;
	//				}
	//			}
	//		}
	//
	//		if ((bunchnum = yax_getbunch(sectorqueue[front], YAX_CEILING)) >= 0) {
	//
	//			for (SECTORS_OF_BUNCH(bunchnum, YAX_FLOOR, ns)) {
	//
	//				if (ns >= 0 && !drawingstate[ns] /*&&
	//					polymer_planeinfrustum(&prsectors[ns]->floor, frustum)*/) {
	//
	//					sectorqueue[back++] = ns;
	//					drawingstate[ns] = 1;
	//				}
	//			}
	//		}
	//#endif
	//		i = sec->wallnum - 1;
	//		do
	//		{
	//			if ((queueStatus[sec->wallptr + i] != POLYMER_OCCLUSION_NOTEST) &&
	//				(!drawingstate[wall[sec->wallptr + i].nextsector]))
	//			{
	//				Build3DSector *pvssector = board->GetSector(wall[sec->wallptr + i].nextsector);
	//				if (softwareRasterizer->IsSectorVisible(wall[sec->wallptr + i].nextsector))
	//				{
	//					sectorqueue[back++] = wall[sec->wallptr + i].nextsector;
	//					drawingstate[wall[sec->wallptr + i].nextsector] = 1;
	//				}
	//
	//				queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_NOTEST;
	//			}
	//			else if (queueStatus[sec->wallptr + i] == POLYMER_OCCLUSION_NOTEST)
	//			{
	//				//bglDeleteQueriesARB(1, &queryid[sec->wallptr + i]);
	//				queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_NOTEST;
	//			}
	//		} while (--i >= 0);
	//
	//		front++;
	//	}

	spritesortcnt = localspritesortcnt;
}