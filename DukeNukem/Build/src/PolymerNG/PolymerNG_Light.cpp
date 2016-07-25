// PolymerNG_Light.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

/*
============================
PolymerNGLightLocal::PolymerNGLightLocal
============================
*/
PolymerNGLightLocal::PolymerNGLightLocal(PolymerNGLightOpts opts, PolymerNGBoard *board)
{
	this->opts_original = this->opts = opts;
	this->polymerNGboard = board;
	this->board = board->GetBoard();

	CalculateLightVisibility();
}

/*
============================
PolymerNGLightLocal::IsPlaneInLight
============================
*/
bool PolymerNGLightLocal::IsPlaneInLight(Build3DPlane* plane)
{
	float           lightpos[3] = { GetOpts()->position[1], -GetOpts()->position[2] / 16.0f, -GetOpts()->position[0] };

	int32_t         i, j, k, l;

	if (!plane->vertcount || plane->buffer == NULL)
		return 0;


	i = 0;

	int radius = GetOpts()->radius * 1000;

	do
	{
		j = k = l = 0;

		do
		{
			if ((&plane->buffer[j].position.x)[i] > (lightpos[i] + radius)) k++;
			if ((&plane->buffer[j].position.x)[i] < (lightpos[i] - radius)) l++;
		} while (++j < plane->vertcount);

		if ((k == plane->vertcount) || (l == plane->vertcount))
			return 0;
	} while (++i < 3);

	return 1;
}


/*
============================
PolymerNGLightLocal::CalculateLightVisibility
============================
*/
void  PolymerNGLightLocal::CalculateLightVisibility()
{
	int32_t         front = 0;
	int32_t         back = 1;
	int16_t			drawingstate[MAXSECTORS];
	int16_t			sectorqueue[MAXSECTORS];

	Bmemset(drawingstate, 0, sizeof(int16_t) * numsectors);
	drawingstate[opts.sector] = 1;

	if (opts.lightType == POLYMERNG_LIGHTTYPE_POINT)
	{
		float3 lightPosition(opts.position[1], -opts.position[2] / 16.0f, -opts.position[0]);

		lightVisibility.sectorInfluences.clear();

		sectorqueue[0] = opts.sector;

		do
		{
			Build3DSector *build3DSector = board->GetSector(sectorqueue[front]);
			tsectortype *sec = (tsectortype *)&sector[sectorqueue[front]];

			if (sec->wallptr >= 0 && build3DSector->boundingbox.intersectsSphere(lightPosition, GetOpts()->radius * 1000))
			{
				int i = 0;
				while (i < sec->wallnum)
				{
					Build3DWall *w = board->GetWall(sec->wallptr + i);

					bool proceedToBack = (IsPlaneInLight(&w->wall) && IsPlaneInLight(&w->over)) || IsPlaneInLight(&w->mask);

					// assume the light hits the middle section if it hits the top and bottom
					if (proceedToBack && wallvisible(opts.position[0], opts.position[1], sec->wallptr + i))
					{
						if ((w->mask.vertcount == 4) &&
							(w->mask.buffer[0].position.y >= w->mask.buffer[3].position.y) &&
							(w->mask.buffer[1].position.y >= w->mask.buffer[2].position.y))
						{
							i++;
							continue;
						}

						if ((wall[sec->wallptr + i].nextsector >= 0) &&
							(!drawingstate[wall[sec->wallptr + i].nextsector])) {
							drawingstate[wall[sec->wallptr + i].nextsector] = 1;
							sectorqueue[back] = wall[sec->wallptr + i].nextsector;
							back++;
						}
					}

					i++;
				}
				lightVisibility.sectorInfluences.push_back(sectorqueue[front]);
			}
			front++;
		} while (front != back);
	}
	else if (opts.lightType == POLYMERNG_LIGHTTYPE_SPOT) // THIS IS WRONG FOR SPOTLIGHTS!!!
	{
		float3 lightPosition(opts.position[1], -opts.position[2] / 16.0f, -opts.position[0]);

		lightVisibility.sectorInfluences.clear();

		for (int d = 0; d < numsectors; d++)
		{
			Build3DSector *build3DSector = board->GetSector(d);
			tsectortype *sec = (tsectortype *)&sector[d];

			if (sec->wallptr < 0)
				continue;

			if (!build3DSector->boundingbox.intersectsSphere(lightPosition, GetOpts()->radius * 1000))
				continue;


			lightVisibility.sectorInfluences.push_back(d);
		}
	}
}

/*
============================
PolymerNGLightLocal::DoesLightInfluenceSector
============================
*/
bool PolymerNGLightLocal::DoesLightInfluenceSector(int sectorId)
{
	for (int i = 0; i < lightVisibility.sectorInfluences.size(); i++)
	{
		if (lightVisibility.sectorInfluences[i] == sectorId)
			return true;
	}

	return false;
}

/*
============================
PolymerNGLightLocal::CreateFrustumFromModelViewMatrix
============================
*/
void PolymerNGLightLocal::CreateFrustumFromModelViewMatrix(float *modelViewProjectionMatrix, float* frustum)
{
	int32_t i;
	i = 0;
	do
	{
		uint32_t ii = i << 2, iii = (i << 2) + 3;

		frustum[i] = modelViewProjectionMatrix[iii] + modelViewProjectionMatrix[ii];               // left
		frustum[i + 4] = modelViewProjectionMatrix[iii] - modelViewProjectionMatrix[ii];           // right
		frustum[i + 8] = modelViewProjectionMatrix[iii] - modelViewProjectionMatrix[ii + 1];     // top
		frustum[i + 12] = modelViewProjectionMatrix[iii] + modelViewProjectionMatrix[ii + 1];    // bottom
		frustum[i + 16] = modelViewProjectionMatrix[iii] - modelViewProjectionMatrix[ii + 2];    // far
	} while (++i < 4);
}

/*
============================
PolymerNGLightLocal::IsPlaneInFrustum
============================
*/
bool PolymerNGLightLocal::IsPlaneInFrustum(Build3DPlane *plane, float* frustum)
{
	int32_t         i, j, k = -1;
	i = 4;

	do
	{
		int32_t ii = i * 4;
		j = k = plane->vertcount - 1;

		do
		{
			k -= ((frustum[ii + 0] * plane->buffer[j].position.x +
				frustum[ii + 1] * plane->buffer[j].position.y +
				frustum[ii + 2] * plane->buffer[j].position.z +
				frustum[ii + 3]) < 0.f);
		} while (j--);

		if (k == -1)
			return false; // OUT !
	} while (i--);

	return true;
}

/*
============================
PolymerNGLightLocal::PrepareShadows
============================
*/
void PolymerNGLightLocal::PrepareShadows(Build3DSprite *prsprites, int numSprites, float4x4 modelViewMatrix)
{
	bool smpFrame = !renderer.GetCurrentFrameNum();
	if (opts.lightType == POLYMERNG_LIGHTTYPE_POINT)
	{
		float3 lightPosition(opts.position[1], -opts.position[2] / 16.0f, -opts.position[0]);

		Build3DPlane **boardPlanes = board->GetGlobalPlaneList();

		for (int i = 0; i < 6; i++)
		{
			float4x4 lightMatrix = float4x4Identity();
			lightMatrix = lightMatrix * float4x4Translation(-lightPosition.x, -lightPosition.y, -lightPosition.z);
			lightMatrix = float4x4Scale(1.0f / 1000.0f, 1.0f / 1000.0f, 1.0f / 1000.0f) * lightMatrix;

			glhPerspectivef2(&lightProjectionMatrix.r0.x, 90.0f, 1, 0.01f, GetOpts()->radius);

			float4x4 sliceProjectionMatrix = lightProjectionMatrix * renderer.pointLightShadowFaceMatrix[i];
			shadowPasses[i].shadowViewProjectionMatrix = sliceProjectionMatrix * lightMatrix;
			shadowPasses[i].shadowViewMatrix = lightMatrix;
			shadowPasses[i].shadowProjectionMatrix = lightProjectionMatrix;

			shadowPasses[i].shadowOccluders[smpFrame].clear();
			
			// Find all sprites that this light can influence.
			// Despite the bad naming on my part, drawsprites just agros a list of visible sprites.

			for (int d = 0; d < numSprites; d++)
			{
				// Todo: Add wall sprite alpha support!
				if (!prsprites[d].cacheModel)
					continue;

				PolymerNGShadowOccluder occluder;
				occluder.sprite = prsprites[d];
				occluder.sprite.ViewMatrix = lightMatrix;

				float4x4 modelViewMatrix = lightMatrix * occluder.sprite.modelMatrix;
				occluder.sprite.modelViewProjectionMatrix = sliceProjectionMatrix * modelViewMatrix;
				shadowPasses[i].shadowOccluders[smpFrame].push_back(occluder);
			}

			// Find all the sectors this light can influence. 
			for (int d = 0; d < lightVisibility.sectorInfluences.size(); d++)
			{
				int sectorId = lightVisibility.sectorInfluences[d];
				Build3DSector *build3DSector = board->GetSector(sectorId);
				tsectortype *sec = (tsectortype *)&sector[sectorId];

				{
					PolymerNGShadowOccluder occluder;
					occluder.plane = &build3DSector->ceil;
					shadowPasses[i].shadowOccluders[smpFrame].push_back(occluder);
				}

				{
					PolymerNGShadowOccluder occluder;
					occluder.plane = &build3DSector->floor;
					shadowPasses[i].shadowOccluders[smpFrame].push_back(occluder);
				}

				for (int w = 0; w < sec->wallnum; w++)
				{
					int wallNum = sec->wallptr + w;
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

					bool parralaxFloor = (neighborSector != NULL && neighborSector && build3DSector->IsFloorParalaxed() && neighborSector->IsFloorParalaxed());
					if ((wall->underover & 1) && (!parralaxFloor || searchit == 2))
					{
						PolymerNGShadowOccluder occluder;
						occluder.plane = &wall->wall;
						shadowPasses[i].shadowOccluders[smpFrame].push_back(occluder);
					}
					bool parralaxCeiling = (neighborSector != NULL && neighborSector && build3DSector->IsCeilParalaxed() && neighborSector->IsCeilParalaxed());
					if ((wall->underover & 2) && (!parralaxCeiling || searchit == 2))
					{
						PolymerNGShadowOccluder occluder;
						occluder.plane = &wall->over;
						shadowPasses[i].shadowOccluders[smpFrame].push_back(occluder);
					}

					//if ((::wall[wallNum].cstat & 32) && (::wall[wallNum].nextsector >= 0))
					if ((::wall[wallNum].cstat & 48) == 16)
					{
						PolymerNGShadowOccluder occluder;
						occluder.plane = &wall->mask;
						shadowPasses[i].shadowOccluders[smpFrame].push_back(occluder);
					}
				}
			}
		}
	}
	else if (opts.lightType == POLYMERNG_LIGHTTYPE_SPOT)
	{
		float4 lightPosition(opts.position[1], -opts.position[2] / 16.0f, -opts.position[0], 1.0f);

		float radius = (float)(GetOpts()->radius) / (2048.0f / 360.0f);
		float ang = (float)(GetOpts()->angle) / (2048.0f / 360.0f);
		float horizang = (float)(-getangle(128, GetOpts()->horiz - 100)) / (2048.0f / 360.0f);

		// Create our light transformation matrix.
		float4x4 lightMatrix = float4x4Identity();
		_math_matrix_rotate(lightMatrix, horizang, 1.0f, 0.0f, 0.0f);
		_math_matrix_rotate(lightMatrix, ang, 0.0f, 1.0f, 0.0f);
		lightMatrix = lightMatrix * float4x4Translation(-lightPosition.x, -lightPosition.y, -lightPosition.z);
		lightMatrix = lightMatrix * float4x4Scale(1.0f / 1000.0f, 1.0f / 1000.0f, 1.0f / 1000.0f);

		shadowPasses[0].shadowViewMatrix = lightMatrix;

		// Create our projection matrix.
		glhPerspectivef2(&lightProjectionMatrix.r0.x, radius * 2, 1, 0.1f, GetOpts()->range / 1000.0f);

		shadowPasses[0].shadowViewProjectionMatrix = lightMatrix * lightProjectionMatrix;
		shadowPasses[0].shadowViewMatrix = lightMatrix;
		shadowPasses[0].shadowProjectionMatrix = lightProjectionMatrix;

		// Lifted directly from Polymer(modified slightly to fit NG); best to keep the math the same as Polymer in order to maintain compaitibility. 
		float4 transformedLightPosition = modelViewMatrix * lightPosition;
		
		float sinang, cosang, sinhorizang, coshorizangs;
		float4 indir;

		cosang = (float)(sintable[(-GetOpts()->angle + 1024) & 2047]) / 16383.0f;
		sinang = (float)(sintable[(-GetOpts()->angle + 512) & 2047]) / 16383.0f;
		coshorizangs = (float)(sintable[(getangle(128, GetOpts()->horiz - 100) + 1024) & 2047]) / 16383.0f;
		sinhorizang = (float)(sintable[(getangle(128, GetOpts()->horiz - 100) + 512) & 2047]) / 16383.0f;

		indir.x = lightPosition.x + sinhorizang * cosang;
		indir.y = lightPosition.y - coshorizangs;
		indir.z = lightPosition.z - sinhorizang * sinang;

		float4 transformedDirection = modelViewMatrix * indir;
		transformedDirection.x -= transformedLightPosition.x;
		transformedDirection.y -= transformedLightPosition.y;
		transformedDirection.z -= transformedLightPosition.z;

		indir.x = (float)(sintable[(((int)GetOpts()->radius) + 512) & 2047]) / 16383.0f;
		indir.y = (float)(sintable[(GetOpts()->faderadius + 512) & 2047]) / 16383.0f;
		indir.z = 1.0 / (indir.x - indir.y);

		shadowPasses[0].spotdir = transformedDirection;
		shadowPasses[0].spotRadius = indir;

		for (int d = 0; d < lightVisibility.sectorInfluences.size(); d++)
		{
			int sectorId = lightVisibility.sectorInfluences[d];
			Build3DSector *build3DSector = board->GetSector(sectorId);
			tsectortype *sec = (tsectortype *)&sector[sectorId];

			{
				PolymerNGShadowOccluder occluder;
				occluder.plane = &build3DSector->ceil;
				shadowPasses[0].shadowOccluders[smpFrame].push_back(occluder);
			}

			{
				PolymerNGShadowOccluder occluder;
				occluder.plane = &build3DSector->floor;
				shadowPasses[0].shadowOccluders[smpFrame].push_back(occluder);
			}

			for (int w = 0; w < sec->wallnum; w++)
			{
				int wallNum = sec->wallptr + w;
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

				bool parralaxFloor = (neighborSector != NULL && neighborSector && build3DSector->IsFloorParalaxed() && neighborSector->IsFloorParalaxed());
				if ((wall->underover & 1) && (!parralaxFloor || searchit == 2))
				{
					PolymerNGShadowOccluder occluder;
					occluder.plane = &wall->wall;
					shadowPasses[0].shadowOccluders[smpFrame].push_back(occluder);
				}
				bool parralaxCeiling = (neighborSector != NULL && neighborSector && build3DSector->IsCeilParalaxed() && neighborSector->IsCeilParalaxed());
				if ((wall->underover & 2) && (!parralaxCeiling || searchit == 2))
				{
					PolymerNGShadowOccluder occluder;
					occluder.plane = &wall->over;
					shadowPasses[0].shadowOccluders[smpFrame].push_back(occluder);
				}

				//if ((::wall[wallNum].cstat & 32) && (::wall[wallNum].nextsector >= 0))
				if ((::wall[wallNum].cstat & 48) == 16)
				{
					PolymerNGShadowOccluder occluder;
					occluder.plane = &wall->mask;
					shadowPasses[0].shadowOccluders[smpFrame].push_back(occluder);
				}
			}
		}
	}
}
/*
============================
PolymerNGLightLocal::MoveLightsInSector
============================
*/
void PolymerNGBoard::MoveLightsInSector(int sectorNum, float deltax, float deltay)
{
	for (int i = 0; i < mapLights.size(); i++)
	{
		if (mapLights[i]->GetOpts()->sector != sectorNum)
			continue;

		mapLights[i]->GetOpts()->position[0] = mapLights[i]->GetOriginalOpts()->position[0] + deltax;
		mapLights[i]->GetOpts()->position[1] = mapLights[i]->GetOriginalOpts()->position[1] + deltay;

		// Update the light visibility.
		mapLights[i]->CalculateLightVisibility();
	}
}

/*
============================
PolymerNGLightLocal::SetAmbientLightForSector
============================
*/
void PolymerNGBoard::SetAmbientLightForSector(int sectorNum, int ambientNum)
{
	board->GetSector(sectorNum)->ambientSectorId = ambientNum;
}

/*
============================
PolymerNGLightLocal::FindVisibleLightsForScene
============================
*/
void PolymerNGBoard::FindVisibleLightsForScene(Build3DSprite *prsprites, int numSprites, PolymerNGLightLocal **lights, int &numVisibleLights, float4x4 modelViewMatrix)
{
	// Tick all the lights
	for (int i = 0; i < mapLights.size(); i++)
	{
		if (mapLights[i]->GetOpts()->LightTick)
		{
			if (!mapLights[i]->GetOpts()->LightTick(mapLights[i], mapLights[i]->GetOpts()))
			{
				RemoveLightFromCurrentBoard(mapLights[i]);
			}
		}
	}

	// Check to see if a light is bound to a sprite, if it is check to see if the position needs to be updated.
	for (int i = 0; i < mapLights.size(); i++)
	{
		SPRITETYPE *owner = mapLights[i]->GetOpts()->spriteOwner;

		if (owner)
		{
			// Remove the light if the owner has been deleted.
			if (owner->statnum == MAXSTATUS)
			{
				RemoveLightFromCurrentBoard(mapLights[i]);
				continue;
			}

			if (owner->x != mapLights[i]->GetOpts()->position[0] || owner->y != mapLights[i]->GetOpts()->position[1] || owner->z != mapLights[i]->GetOpts()->position[2])
			{
				mapLights[i]->GetOpts()->position[0] = owner->x;
				mapLights[i]->GetOpts()->position[1] = owner->y;
				mapLights[i]->GetOpts()->position[2] = owner->z;

				// Update the light visibility.
				mapLights[i]->CalculateLightVisibility();
			}
		}
	}

	numVisibleLights = 0;
	for (int i = 0; i < mapLights.size(); i++)
	{
		if (numVisibleLights >= MAX_VISIBLE_LIGHTS)
		{
			initprintf("PolymerNGBoard::FindVisibleLightsForScene: Too many lights in view at once!\n");
			return;
		}

		bool isLightVisible = false;
		for (int d = 0; d < numVisibleSectors; d++)
		{
			if (mapLights[i]->DoesLightInfluenceSector(visibleSectorsArray[d]))
			{
				isLightVisible = true;
				break;
			}
		}

		if (!isLightVisible)
			continue;

		lights[numVisibleLights] = mapLights[i];

		if (lights[numVisibleLights]->GetOpts()->castShadows)
		{
			lights[numVisibleLights]->PrepareShadows(prsprites, numSprites, modelViewMatrix);
		}

		numVisibleLights++;
	}
}