// BaseModel.cpp
//

#include "../PolymerNG.h"

BaseModel::BaseModel()
{
	rhiVertexBufferStatic = NULL;
	numUpdateQueues = 0;
	dynamicBufferDirty[0] = false;
	dynamicBufferDirty[1] = false;
	//meshVertexes.reserve(300);
}

void BaseModel::AllocateBuffer(int size)
{
	meshVertexes.resize(size);
}

int BaseModel::UpdateBuffer(int startPosition, int numVertexes, Build3DVertex *vertexes, int sectorNum, bool cpuUpdateOnly)
{
	Build3DVertex *vertexpool = &meshVertexes[startPosition];
	memcpy(vertexpool, vertexes, sizeof(Build3DVertex) * numVertexes);

	if (cpuUpdateOnly)
		return startPosition;

	if (sectorNum != -1)
	{
		for (int i = 0; i < numVertexes; i++)
		{
			vertexpool[i].uv.z = sectorNum;
		}
	}

	ModelUpdateQueuedItem *queuedItem = &modelUpdateQueue[numUpdateQueues++];
	queuedItem->startPosition = startPosition;
	queuedItem->numVertexes = numVertexes;

	dynamicBufferDirty[0] = true;
	dynamicBufferDirty[1] = true;

	return startPosition;
}

int BaseModel::AddIndexesToBuffer(int numIndexes, unsigned short *indexes, int startVertexPosition)
{
	int startPosition = meshIndexes.size();
	meshIndexes.resize(startPosition + numIndexes);
	unsigned int *indexPool = &meshIndexes[startPosition];
//	memcpy(indexPool, indexes, sizeof(unsigned short) * numIndexes);
	for (int i = 0; i < numIndexes; i++)
	{
		indexPool[i] = indexes[i] + startVertexPosition;
	}

	return startPosition;
}

int BaseModel::AddIndexesToBuffer(int numIndexes, unsigned int *indexes, int startVertexPosition)
{
	int startPosition = meshIndexes.size();
	meshIndexes.resize(startPosition + numIndexes);
	unsigned int *indexPool = &meshIndexes[startPosition];
	//	memcpy(indexPool, indexes, sizeof(unsigned short) * numIndexes);
	for (int i = 0; i < numIndexes; i++)
	{
		indexPool[i] = indexes[i] + startVertexPosition;
	}

	return startPosition;
}

int BaseModel::AddVertexesToBuffer(int numVertexes, Build3DVertex *vertexes, int sectorNum)
{
	int startPosition = meshVertexes.size();
	meshVertexes.resize(startPosition + numVertexes);
	Build3DVertex *vertexpool = &meshVertexes[startPosition];

	memcpy(vertexpool, vertexes, sizeof(Build3DVertex) * numVertexes);

	if (sectorNum != -1)
	{
		for (int i = 0; i < numVertexes; i++)
		{
			vertexpool[i].uv.z = sectorNum;
		}
	}

	//for (int i = numVertexes - 1; i >= 0; i--)
	//{
	//	meshVertexes.push_back(vertexes[i]);
	//}

	return startPosition;
}