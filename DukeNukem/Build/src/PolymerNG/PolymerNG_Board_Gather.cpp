// PolymerNG_Board_Gather.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

//#define DISABLE_CULLING 1

int32_t numDisplayedRenderPlanes = 0;

enum PolymerNGOcclusionTest
{
	POLYMER_OCCLUSION_FAIL = 0,
	POLYMER_OCCLUSION_PASS
};


/*
=============
PolymerNGBoard::InitOcclusion
=============
*/
void PolymerNGBoard::InitOcclusion()
{
	visibilityEngine->CreateOcclusionFromBoard();
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
	//Build3D::CalculateFogForPlane(plane->tileNum, plane->shadeNum, plane->visibility, plane->paletteNum, plane);
	memcpy((void *)&renderWorldTask.renderplanes[renderWorldTask.numRenderPlanes++], &plane, sizeof(Build3DPlane *));
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
void PolymerNGBoard::FindVisibleSectors(BuildRenderThreadTaskRenderWorld &renderWorldTask, const Math::Matrix4 &modelViewProjectionMatrix, const Math::Matrix4 &modelViewMatrix, const Math::Matrix4 &projectionMatrix, int16_t dacursectnum)
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


	std::vector<int32_t> visibleSectorsArray;
	{
		float modelViewMatrixRaw[16];
		float projectionMatrixRaw[16];
		((Math::Matrix4)modelViewMatrix).GetFloat4x4((DirectX::XMFLOAT4X4 *)&modelViewMatrixRaw[0]);
		((Math::Matrix4)projectionMatrix).GetFloat4x4((DirectX::XMFLOAT4X4 *)&projectionMatrixRaw[0]);

		visibilityEngine->FindVisibleSectors(renderWorldTask, &modelViewMatrixRaw[0], &projectionMatrixRaw[0], visibleSectorsArray);
	}

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

	int numVisibleSectors = visibleSectorsArray.size();
	if (numVisibleSectors == 0)
		return;

	int32_t *visibleSectors = &visibleSectorsArray[0];


	for (int d = 0; d < numVisibleSectors; d++)
	{
		int sectorNum = visibleSectorsArray[d];

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
			//if (wallvisible(globalposx, globalposy, wallNum)) // No reason to do this.
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

				bool parralaxCeiling = (neighborSector != NULL && neighborSector && sector->IsCeilParalaxed() && neighborSector->IsCeilParalaxed());
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


	numDisplayedRenderPlanes = renderWorldTask.numRenderPlanes;

	spritesortcnt = localspritesortcnt;

}