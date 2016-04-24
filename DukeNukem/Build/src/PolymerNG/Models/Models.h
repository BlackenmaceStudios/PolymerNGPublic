// Models.h
//

#pragma once

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

	void					UpdateBuffer(int startPosition, int numVertexes, Build3DVertex *vertexes);

	int						AddVertexesToBuffer(int numVertexes, Build3DVertex *vertexes);
	int						AddIndexesToBuffer(int numIndexes, unsigned short *indexes);

	std::vector<ModelUpdateQueuedItem> geoUpdateQueue[2];

	std::vector<Build3DVertex> meshVertexes;
	std::vector<unsigned short> meshIndexes;

	BuildRHIMesh			*rhiVertexBufferStatic;
	BuildRHIMesh			*rhiVertexBufferDynamic[2];
};