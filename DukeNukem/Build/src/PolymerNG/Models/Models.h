// Models.h
//

#pragma once

class BuildRHIMesh;

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

	std::vector<Build3DVertex> meshVertexes;
	std::vector<unsigned short> meshIndexes;

	BuildRHIMesh			*rhiVertexBufferStatic;
};