// PolymerNG_Occlusion.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

#define DISABLE_CULLING 1

enum PolymerNGOcclusionTest
{
	POLYMER_OCCLUSION_NOTEST = 0,
	POLYMER_OCCLUSION_TEST
};

struct BoundingBox2D
{
	float min[2];
	float max[2];
	float depth;
	bool visible;
};

Build3DVector4 ConvertVertexToViewportSpace(Math::Vector4 position, int width, int height)
{
	Build3DVector4 p;

	p.x = position.GetX() / position.GetW();
	p.y = position.GetY() / position.GetW();
	p.z = position.GetZ() / position.GetW();

	p.x = (int)(0.5f + ((p.x + 1) * 0.5f * width));
	p.y = (int)(0.5f + ((1 - p.y) * 0.5f * height));

	return p;
}

OccluderPoint ConvertBuildVecterToOccluder(Build3DVector4 vector)
{
	OccluderPoint point;
	point.x = vector.x;
	point.y = vector.y;
	point.depth = vector.z;
	return point;
}

bool ProjectBoundingBox(int width, int height, float min[3], float max[3], const Math::Matrix4 &modelViewProjectionMatrix, BoundingBox2D &box2D)
{
	Math::Vector3 corners[8];

	corners[0].SetXYZ(min[0], min[1], min[2]);
	corners[1].SetXYZ(min[0], min[1], max[2]);

	corners[2].SetXYZ(min[0], max[1], min[2]);
	corners[3].SetXYZ(min[0], max[1], max[2]);

	corners[4].SetXYZ(max[0], min[1], min[2]);
	corners[5].SetXYZ(max[0], min[1], max[2]);

	corners[6].SetXYZ(max[0], max[1], min[2]);
	corners[7].SetXYZ(max[0], max[1], max[2]);


	Build3DVector4 projVertices[8];
	
	for (int i = 0; i < 8; i++)
	{
		Math::Vector4 pOut = modelViewProjectionMatrix * corners[i]; 
		if (pOut.GetW() < 0.0f)
		{
			return true;
		}
		projVertices[i] = ConvertVertexToViewportSpace(pOut, width, height);
	}

	float min2D[2] = { FLT_MAX, FLT_MAX };
	float max2D[2] = { FLT_MIN, FLT_MIN };

	float minDepth = FLT_MAX;
	for (int i = 0; i < 8; i++)
	{
		if (projVertices[i].x < min2D[0])
		{
			min2D[0] = projVertices[i].x;
		}
		if (projVertices[i].y < min2D[1])
		{
			min2D[1] = projVertices[i].y;
		}
		if (projVertices[i].x > max2D[0])
		{
			max2D[0] = projVertices[i].x;
		}
		if (projVertices[i].y > max2D[1])
		{
			max2D[1] = projVertices[i].y;
		}

		if (projVertices[i].z < minDepth)
		{
			minDepth = projVertices[i].z;
		}
	}

	if (min2D[0] < 0.0f) min2D[0] = 0.0f;
	if (min2D[1] < 0.0f) min2D[1] = 0.0f;
	if (max2D[0] >= width) max2D[0] = width - 1;
	if (max2D[1] >= height) max2D[1] = height - 1;

	if (max2D[0] - min2D[0] < 1.0f) return true;
	if (max2D[1] - min2D[1] < 1.0f) return true;


	box2D.min[0] = min[0];
	box2D.min[1] = min[1];
	box2D.max[0] = max[0];
	box2D.max[1] = max[1];
	box2D.depth = minDepth;
	return false;
}

/*
=============
PolymerNGBoard::InitOcclusion
=============
*/
void PolymerNGBoard::InitOcclusion()
{
	initprintf("Initializing occlusion engine for map\n");
	occlusionHandle = InitializeOcclusionEngineDefault(1920 / 6, 1080 / 6);
}

/*
=============
PolymerNGBoard::RenderOccluderFromPlane
=============
*/
void PolymerNGBoard::RenderOccluderFromPlane(const Math::Matrix4 &modelViewProjectionMatrix, const Build3DPlane *plane)
{
#if DISABLE_CULLING
	return;
#endif
	for (int i = 0; i < plane->indicescount; i += 3)
	{
		int32_t index0 = plane->indices[i + 0];
		int32_t index1 = plane->indices[i + 1];
		int32_t index2 = plane->indices[i + 2];

		OccluderData occluderData[2];
		Math::Vector4 vertex0(plane->buffer[index0].position.x, plane->buffer[index0].position.y, plane->buffer[index0].position.z, 1.0f);
		Math::Vector4 vertex1(plane->buffer[index1].position.x, plane->buffer[index1].position.y, plane->buffer[index1].position.z, 1.0f);
		Math::Vector4 vertex2(plane->buffer[index2].position.x, plane->buffer[index2].position.y, plane->buffer[index2].position.z, 1.0f);

		// Transform the vertexes into screen space.
		vertex0 = modelViewProjectionMatrix * vertex0;
		vertex1 = modelViewProjectionMatrix * vertex1;
		vertex2 = modelViewProjectionMatrix * vertex2;

		occluderData[0].numberOfPoints = 3;
		occluderData[0].points[0] = ConvertBuildVecterToOccluder(ConvertVertexToViewportSpace(vertex0, 1920 / 6, 1080 / 6));
		occluderData[0].points[1] = ConvertBuildVecterToOccluder(ConvertVertexToViewportSpace(vertex1, 1920 / 6, 1080 / 6));
		occluderData[0].points[2] = ConvertBuildVecterToOccluder(ConvertVertexToViewportSpace(vertex2, 1920 / 6, 1080 / 6));

		occlusionHandle->addOccluders(occluderData, 1);
	}
}

/*
=============
PolymerNGBoard::AddRenderPlaneToDrawList
=============
*/
void PolymerNGBoard::AddRenderPlaneToDrawList(BuildRenderThreadTaskRenderWorld &renderWorldTask, Build3DPlane *plane)
{
	renderWorldTask.renderplanes[renderWorldTask.numRenderPlanes++] = plane;
}

/*
=============
PolymerNGBoard::IsSectorVisible
=============
*/
bool PolymerNGBoard::IsSectorVisible(const Math::Matrix4 &modelViewProjectionMatrix, Build3DSector *sector)
{
#if DISABLE_CULLING
	return true;
#endif
	OccludeeData occludee;

	float sectorSizeMin[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
	float sectorSizeMax[3] = { FLT_MIN, FLT_MIN, FLT_MIN };

	for (int i = 0; i < sector->floor.vertcount; i++)
	{
		if (sector->floor.buffer[i].position.x < sectorSizeMin[0]) sectorSizeMin[0] = sector->floor.buffer[i].position.x;
		if (sector->floor.buffer[i].position.y < sectorSizeMin[1]) sectorSizeMin[1] = sector->floor.buffer[i].position.y;
		if (sector->floor.buffer[i].position.z < sectorSizeMin[2]) sectorSizeMin[2] = sector->floor.buffer[i].position.z;
		if (sector->floor.buffer[i].position.x > sectorSizeMax[0]) sectorSizeMax[0] = sector->floor.buffer[i].position.x;
		if (sector->floor.buffer[i].position.y > sectorSizeMax[1]) sectorSizeMax[1] = sector->floor.buffer[i].position.y;
		if (sector->floor.buffer[i].position.z > sectorSizeMax[2]) sectorSizeMax[2] = sector->floor.buffer[i].position.z;
	}

	for (int i = 0; i < sector->ceil.vertcount; i++)
	{
		if (sector->ceil.buffer[i].position.x < sectorSizeMin[0]) sectorSizeMin[0] = sector->ceil.buffer[i].position.x;
		if (sector->ceil.buffer[i].position.y < sectorSizeMin[1]) sectorSizeMin[1] = sector->ceil.buffer[i].position.y;
		if (sector->ceil.buffer[i].position.z < sectorSizeMin[2]) sectorSizeMin[2] = sector->ceil.buffer[i].position.z;
		if (sector->ceil.buffer[i].position.x > sectorSizeMax[0]) sectorSizeMax[0] = sector->ceil.buffer[i].position.x;
		if (sector->ceil.buffer[i].position.y > sectorSizeMax[1]) sectorSizeMax[1] = sector->ceil.buffer[i].position.y;
		if (sector->ceil.buffer[i].position.z > sectorSizeMax[2]) sectorSizeMax[2] = sector->ceil.buffer[i].position.z;
	}

	BoundingBox2D box2D;
	if (ProjectBoundingBox(1920 / 6, 1080 / 6, sectorSizeMin, sectorSizeMax, modelViewProjectionMatrix, box2D))
		return true;

	occludee.depth = box2D.depth;

	occludee.boundingBox.xMin = (int)box2D.min[0];
	occludee.boundingBox.yMin = (int)box2D.min[1];
	occludee.boundingBox.xMax = (int)box2D.max[0];
	occludee.boundingBox.yMax = (int)box2D.max[1];

	return occlusionHandle->testOccludeeVisibility(occludee);
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

	occlusionHandle->clear();
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

	while (front != back)
	{
		sec = (tsectortype *)&sector[sectorqueue[front]];

		// Draw the sectors.
		Build3DSector *sector = board->GetSector(sectorqueue[front]);

		PokeSector(sectorqueue[front]);

		if (!sector->IsCeilParalaxed() && yax_getbunch(sectorqueue[front], YAX_CEILING) < 0)
		{
			AddRenderPlaneToDrawList(renderWorldTask, &sector->ceil);
		}

		if (!sector->IsFloorParalaxed() && yax_getbunch(sectorqueue[front], YAX_FLOOR) < 0)
		{
			AddRenderPlaneToDrawList(renderWorldTask, &sector->floor);
		}

		ScanSprites(sectorqueue[front], localtsprite, &localspritesortcnt);

		//polymer_pokesector(sectorqueue[front]);
		//polymer_drawsector(sectorqueue[front], FALSE);
		
		doquery = 1;

		i = sec->wallnum - 1;
		do
		{
			// if we have a level boundary somewhere in the sector,
			// consider these walls as visportals
			if (wall[sec->wallptr + i].nextsector < 0)
				doquery = 1;
		} while (--i >= 0);

		i = sec->wallnum - 1;
		while (i >= 0)
		{
			if ((wall[sec->wallptr + i].nextsector >= 0) &&
				(wallvisible(globalposx, globalposy, sec->wallptr + i)))
			{
				if ((board->GetWall(sec->wallptr + i)->mask.vertcount == 4) &&
					!(board->GetWall(sec->wallptr + i)->underover & 4) &&
					!(board->GetWall(sec->wallptr + i)->underover & 8))
				{
					// early exit for closed sectors
					Build3DWall         *w = board->GetWall(sec->wallptr + i);


					if ((w->mask.buffer[0].position.y >= w->mask.buffer[3].position.y) &&
						(w->mask.buffer[1].position.y >= w->mask.buffer[2].position.y))
					{
						i--;
						continue;
					}
				}

				if ((wall[sec->wallptr + i].cstat & 48) == 16)
				{
					int pic = wall[sec->wallptr + i].overpicnum;

					//if (tilesiz[pic].x > 0 && tilesiz[pic].y > 0)
					//	localmaskwall[localmaskwallcnt++] = sec->wallptr + i;
				}

				//if (!depth && (overridematerial & prprogrambits[PR_BIT_MIRROR_MAP].bit) &&
				//	wall[sec->wallptr + i].overpicnum == 560 &&
				//	wall[sec->wallptr + i].cstat & 32)
				//{
				//	mirrorlist[mirrorcount].plane = &prwalls[sec->wallptr + i]->mask;
				//	mirrorlist[mirrorcount].sectnum = sectorqueue[front];
				//	mirrorlist[mirrorcount].wallnum = sec->wallptr + i;
				//	mirrorcount++;
				//}

				{
					
					{
						float pos[3], sqdist;
						int32_t oldoverridematerial;

						pos[0] = (float)globalposy;
						pos[1] = -(float)(globalposz) / 16.0f;
						pos[2] = -(float)globalposx;

						sqdist = board->GetWall(sec->wallptr + i)->mask.plane[0] * pos[0] +
								board->GetWall(sec->wallptr + i)->mask.plane[1] * pos[1] +
								board->GetWall(sec->wallptr + i)->mask.plane[2] * pos[2] +
								board->GetWall(sec->wallptr + i)->mask.plane[3];

						// hack to avoid occlusion querying portals that are too close to the viewpoint
						// this is needed because of the near z-clipping plane;
						//if (sqdist < 100)
						//	queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_NOTEST;
						//else 
						{
							queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_TEST;

							Build3DWall         *w;

							w = board->GetWall(sec->wallptr + i);


							if ((w->underover & 4) && (w->underover & 1))
							{
								RenderOccluderFromPlane(modelViewProjectionMatrix, &w->wall);
							}

							RenderOccluderFromPlane(modelViewProjectionMatrix, &w->mask);
							
							if ((w->underover & 8) && (w->underover & 2))
							{
								RenderOccluderFromPlane(modelViewProjectionMatrix, &w->over);
							}
						}
					}
					//else
					//	queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_NOTEST;
				}
			}

			i--;
		}

		// Cram as much CPU or GPU work as we can between queuing the
		// occlusion queries and reaping them.
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
					AddRenderPlaneToDrawList(renderWorldTask, &wall->wall);
				}

				bool parralaxCeiling =  (neighborSector != NULL && neighborSector && sector->IsCeilParalaxed() && neighborSector->IsCeilParalaxed());
				if ((wall->underover & 2) && (!parralaxCeiling || searchit == 2))
				{
					AddRenderPlaneToDrawList(renderWorldTask, &wall->over);
				}

				if ((::wall[wallNum].cstat & 32) && (::wall[wallNum].nextsector >= 0))
				{
					AddRenderPlaneToDrawList(renderWorldTask, &wall->mask);
				}
			}
		} while (--i >= 0);
#ifdef YAX_ENABLE
		// queue ROR neighbors
		if ((bunchnum = yax_getbunch(sectorqueue[front], YAX_FLOOR)) >= 0) {

			for (SECTORS_OF_BUNCH(bunchnum, YAX_CEILING, ns)) {

				if (ns >= 0 && !drawingstate[ns] /*&&
					polymer_planeinfrustum(&prsectors[ns]->ceil, frustum)*/) {

					sectorqueue[back++] = ns;
					drawingstate[ns] = 1;
				}
			}
		}

		if ((bunchnum = yax_getbunch(sectorqueue[front], YAX_CEILING)) >= 0) {

			for (SECTORS_OF_BUNCH(bunchnum, YAX_FLOOR, ns)) {

				if (ns >= 0 && !drawingstate[ns] /*&&
					polymer_planeinfrustum(&prsectors[ns]->floor, frustum)*/) {

					sectorqueue[back++] = ns;
					drawingstate[ns] = 1;
				}
			}
		}
#endif
		i = sec->wallnum - 1;
		do
		{
			if ((queueStatus[sec->wallptr + i] != POLYMER_OCCLUSION_NOTEST) &&
				(!drawingstate[wall[sec->wallptr + i].nextsector]))
			{
				if (IsSectorVisible(modelViewProjectionMatrix, sector))
				{
					sectorqueue[back++] = wall[sec->wallptr + i].nextsector;
					drawingstate[wall[sec->wallptr + i].nextsector] = 1;
				}

				queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_NOTEST;
			}
			else if (queueStatus[sec->wallptr + i] == POLYMER_OCCLUSION_NOTEST)
			{
				//bglDeleteQueriesARB(1, &queryid[sec->wallptr + i]);
				queueStatus[sec->wallptr + i] = POLYMER_OCCLUSION_NOTEST;
			}
		} while (--i >= 0);

		front++;
	}

	spritesortcnt = localspritesortcnt;
}