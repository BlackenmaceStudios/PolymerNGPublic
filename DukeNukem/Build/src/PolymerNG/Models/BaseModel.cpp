// BaseModel.cpp
//

#include "../PolymerNG.h"

BaseModel::BaseModel()
{
	rhiVertexBufferStatic = NULL;
	//meshVertexes.reserve(300);
}

void BaseModel::AllocateBuffer(int size)
{
	meshVertexes.resize(size);
}

void BaseModel::UpdateBuffer(int startPosition, int numVertexes, Build3DVertex *vertexes)
{
	Build3DVertex *vertexpool = &meshVertexes[startPosition];
	memcpy(vertexpool, vertexes, sizeof(Build3DVertex) * numVertexes);
}

int BaseModel::AddIndexesToBuffer(int numIndexes, unsigned short *indexes)
{
	int startPosition = meshIndexes.size();
	meshIndexes.resize(startPosition + numIndexes);
	unsigned short *indexPool = &meshIndexes[startPosition];
	memcpy(indexPool, indexes, sizeof(unsigned short) * numIndexes);
	//for (int i = numVertexes - 1; i >= 0; i--)
	//{
	//	meshVertexes.push_back(vertexes[i]);
	//}

	return startPosition;
}

int BaseModel::AddVertexesToBuffer(int numVertexes, Build3DVertex *vertexes)
{
	int startPosition = meshVertexes.size();
	meshVertexes.resize(startPosition + numVertexes);
	Build3DVertex *vertexpool = &meshVertexes[startPosition];
	memcpy(vertexpool, vertexes, sizeof(Build3DVertex) * numVertexes);
	//for (int i = numVertexes - 1; i >= 0; i--)
	//{
	//	meshVertexes.push_back(vertexes[i]);
	//}

	return startPosition;
}