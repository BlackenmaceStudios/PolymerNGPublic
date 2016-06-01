// CacheModel.cpp
//

#include "../PolymerNG.h"

void CacheModel::Init(const PolymerNGModelCachePayload *payload)
{
	this->payload = payload;
}

void CacheModel::SetTextureForSurface(PolymerNGModelCacheSurfaceDefine *cacheSurfaceDefines)
{
	for (int i = 0; i < payload->modelCacheInfo.numSurfaces; i++)
	{
		PolymerNGMaterial *material = NULL;
		if(cacheSurfaceDefines->materialName[i].size() <= 0)
			material = materialManager.LoadMaterial(cacheSurfaceDefines->materialName[0].c_str());
		else
			material = materialManager.LoadMaterial(cacheSurfaceDefines->materialName[i].c_str());
		surfaces[i].material = material;
	}
}

int CacheModel::GetNumSurfaces() 
{ 
	return payload->modelCacheInfo.numSurfaces; 
}

void CacheModel::BindToBaseMesh(BaseModel *mesh)
{
	if (baseMesh != NULL)
		return;

	for (int i = 0; i < payload->modelCacheInfo.numSurfaces; i++)
	{
		CacheModelSurface *cacheModelSurface = &surfaces[i];

		cacheModelSurface->startVertex = mesh->AddVertexesToBuffer(payload->surfaces[i].numVertexes, &payload->vertexes[payload->surfaces[i].startVertex], -1);
		cacheModelSurface->startIndex = mesh->AddIndexesToBuffer(payload->surfaces[i].numIndexes, &payload->indexes[payload->surfaces[i].startIndex], cacheModelSurface->startVertex);

		cacheModelSurface->numVertexes = payload->surfaces[i].numVertexes;
		cacheModelSurface->numIndexes = payload->surfaces[i].numIndexes;
	}

	this->baseMesh = mesh;
}

void CacheModel::Unload()
{

}