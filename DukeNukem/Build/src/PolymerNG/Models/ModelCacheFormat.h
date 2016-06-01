// ModelCacheFormat.h
//

#pragma once

#define MODELCACHE_PAYLOAD_IDEN "jmModelPayload"

//
// ModelCacheHeader
// 
struct ModelCacheHeader
{
	ModelCacheHeader()
	{
		strcpy(iden, MODELCACHE_PAYLOAD_IDEN);
		numPayloads = 0;
	}
	char iden[14];
	int numPayloads;
};

//
// ModelCachePayloadHeader
//
struct ModelCachePayloadHeader
{
	char modelpath[256];
	int modelWritePosition;
};

//
// ModelCacheSurface
//
struct ModelCacheSurface
{
	char material[128];

	int startVertex;
	int numVertexes;

	int startIndex;
	int numIndexes;
};

//
// ModelCachePayloadInfo
//
struct ModelCachePayloadInfo
{
	int numVertexes;
	int numIndexes;
	int numSurfaces;
	int numFrames;
};

