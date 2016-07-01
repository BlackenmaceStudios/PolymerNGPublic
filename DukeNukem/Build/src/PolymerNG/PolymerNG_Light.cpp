// PolymerNG_Light.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

/*
============================
PolymerNGLightLocal::PolymerNGLightLocal
============================
*/
PolymerNGLightLocal::PolymerNGLightLocal(PolymerNGLightOpts opts, Build3DBoard *board)
{
	this->opts_original = this->opts = opts;
	this->board = board;

	CalculateLightVisibility();
}

/*
============================
PolymerNGLightLocal::CalculateLightVisibility
============================
*/
void  PolymerNGLightLocal::CalculateLightVisibility()
{
	if (opts.lightType == POLYMERNG_LIGHTTYPE_POINT)
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
void PolymerNGLightLocal::PrepareShadows()
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
			lightMatrix = lightMatrix * float4x4Scale(1.0f / 1000.0f, 1.0f / 1000.0f, 1.0f / 1000.0f);

			Math::glhPerspectivef2(&lightProjectionMatrix.r0.x, 90.0f, 1, 0.01f, GetOpts()->radius);

			float4x4 sliceProjectionMatrix = renderer.pointLightShadowFaceMatrix[i] * lightProjectionMatrix;
			shadowPasses[i].shadowViewProjectionMatrix = lightMatrix * sliceProjectionMatrix;
			shadowPasses[i].shadowViewMatrix = lightMatrix;
			shadowPasses[i].shadowProjectionMatrix = lightProjectionMatrix;

			shadowPasses[i].shadowOccluders[smpFrame].clear();
			//for (int d = 0; d < board->GetNumGlobalPlanes(); d++)
			//{
			//	PolymerNGShadowOccluder occluder;
			//	occluder.plane = boardPlanes[d];
			//	shadowPasses[i].shadowOccluders[smpFrame].push_back(occluder);
			//}

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

					if ((::wall[wallNum].cstat & 32) && (::wall[wallNum].nextsector >= 0))
					{
						PolymerNGShadowOccluder occluder;
						occluder.plane = &wall->mask;
						shadowPasses[i].shadowOccluders[smpFrame].push_back(occluder);
					}
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
PolymerNGLightLocal::FindVisibleLightsForScene
============================
*/
void PolymerNGBoard::FindVisibleLightsForScene(PolymerNGLightLocal **lights, int &numVisibleLights)
{
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
			lights[numVisibleLights]->PrepareShadows();
		}

		numVisibleLights++;
	}
}