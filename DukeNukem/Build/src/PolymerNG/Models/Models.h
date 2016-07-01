// Models.h
//

#pragma once

//
// PolymerNGModelCacheSurfaceDefine
//
struct PolymerNGModelCacheSurfaceDefine
{
	std::string materialName[20];
	int32_t modelid;
	float scale;
	int32_t shadeoff;
	float zadd;
	float yoffset;
	int32_t flags;
};

class BuildRHIMesh;

//
// ModelUpdateQueuedItem
//
struct ModelUpdateQueuedItem
{
	int startPosition;
	int numVertexes;
};

//
// BaseModel
//
class BaseModel
{
public:
	BaseModel();
	void					AllocateBuffer(int size);

	int						UpdateBuffer(int startPosition, int numVertexes, Build3DVertex *vertexes, int sectorNum, bool cpuUpdateOnly = false);

	int						AddVertexesToBuffer(int numVertexes, Build3DVertex *vertexes, int sectorNum);
	int						AddIndexesToBuffer(int numIndexes, unsigned short *indexes, int startVertexPosition);
	int						AddIndexesToBuffer(int numIndexes, unsigned int *indexes, int startVertexPosition);

	std::vector<Build3DVertex> meshVertexes;
	std::vector<unsigned int>  meshIndexes;

	BuildRHIMesh			*rhiVertexBufferStatic;

	ModelUpdateQueuedItem   modelUpdateQueue[5000];
	int	numUpdateQueues;

	bool					dynamicBufferDirty[2];
};

#include "ModelCacheFormat.h"
#include "CacheModel.h"
#include "ModelCacheSystem.h"


