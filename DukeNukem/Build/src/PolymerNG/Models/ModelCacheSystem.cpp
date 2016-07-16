// ModelCache.cpp
//

#include "../PolymerNG.h"
#include "../Renderer/Renderer.h"
#include "../../engine_priv.h"

#include "../../../../Third-Party/zlib/zlib.h"

PolymerNGModelCache modelCacheSystem;

PolymerNGModelCache::PolymerNGModelCache()
{
	LoadModelCache();
}

void PolymerNGModelCache::LoadModelCache()
{
	ModelCacheHeader header;
	cacheFile = BuildFile::OpenFile(MODELCACHE_FILENAME, BuildFile::BuildFile_Read);
	if (cacheFile == NULL)
	{
		numPayloads = -1;
		initprintf("Failed to load model cache.\n");
		return;
	}

	totalSizeOfHighQualityAssets = 0;

	// Read in the texture cache header.
	cacheFile->Read(&header, sizeof(ModelCacheHeader));
	payloadHeaders = new ModelCachePayloadHeader[header.numPayloads];
	payloads = new PolymerNGModelCachePayload[header.numPayloads];

	numPayloads = header.numPayloads;

	// Read in all the payload info.
	cacheFile->Read(payloadHeaders, header.numPayloads * sizeof(ModelCachePayloadHeader));

	initprintf("Model Cache has %d payloads\n", numPayloads);
}

void PolymerNGModelCache::BeginLevelLoad()
{
	if (numPayloads == -1)
		return;

	totalSizeOfHighQualityAssets = 0;
	// Free all previous loaded payloads.
	for (int i = 0; i < numPayloads; i++)
	{
		if (loadedmodels[i] != NULL)
		{
			loadedmodels[i]->Unload();
			loadedmodels[i] = NULL;
		}
	}
}

void PolymerNGModelCache::EndLevelLoad(BaseModel *model)
{
	if (numPayloads == -1)
		return;

	// Now upload all the model data to the base model so we can upload it to the GPU.
	int numLoadedModels = 0;
	for (int i = 0; i < MAXTILES; i++)
	{
		if (loadedmodels[i] == NULL)
			continue;

		numLoadedModels++;
		loadedmodels[i]->BindToBaseMesh(model);
	}

	initprintf("--------PolymerNGModelCache::EndLevelLoad---------\n");
	initprintf("..%d loaded models\n", numLoadedModels);
	initprintf("..%dmb of model data\n", totalSizeOfHighQualityAssets >> 20);
	initprintf("----------------------------------------------------\n");
}

//
// PolymerNGModelCache::LoadModelForTile
//
CacheModel *PolymerNGModelCache::LoadModelForTile(int tileNum)
{
	PolymerNGModelCachePayload *currentPayload = tileCacheOverride[tileNum].payload;
	ModelCachePayloadHeader *currentPayloadInfo = tileCacheOverride[tileNum].payloadHeader;

	if (loadedmodels[tileNum] != NULL)
	{
		return loadedmodels[tileNum];
	}

	if (currentPayloadInfo == NULL)
	{
		return NULL;
	}

	if (numPayloads == -1)
	{
		return NULL;
	}

	cacheFile->Seek(currentPayloadInfo->modelWritePosition, SEEK_SET);
	cacheFile->Read(&currentPayload->modelCacheInfo, sizeof(ModelCachePayloadInfo));
	cacheFile->Read(&currentPayload->surfaces[0], currentPayload->modelCacheInfo.numSurfaces * sizeof(ModelCacheSurface));

	currentPayload->vertexes = new Build3DVertex[currentPayload->modelCacheInfo.numVertexes];
	currentPayload->indexes = new unsigned int[currentPayload->modelCacheInfo.numIndexes];

	cacheFile->Read(&currentPayload->vertexes[0], currentPayload->modelCacheInfo.numVertexes * sizeof(Build3DVertex));
	cacheFile->Read(&currentPayload->indexes[0], currentPayload->modelCacheInfo.numIndexes * sizeof(unsigned int));

	totalSizeOfHighQualityAssets += currentPayload->modelCacheInfo.numVertexes * sizeof(Build3DVertex);
	totalSizeOfHighQualityAssets += currentPayload->modelCacheInfo.numIndexes * sizeof(unsigned int);

	loadedmodels[tileNum] = new CacheModel();
	loadedmodels[tileNum]->Init(currentPayload);
	loadedmodels[tileNum]->SetTextureForSurface(&surfaceOverrideDefines[tileNum]);

	return loadedmodels[tileNum];
}

//
// PolymerNGModelCache::DefineTextureForModelSurface
//
void PolymerNGModelCache::DefineTextureForModelSurface(int modelId, int surfaceId, const char *fileName)
{
	surfaceOverrideDefines[modelId].materialName[surfaceId] = fileName;
	//PolymerNGMaterial *material = materialManager.LoadMaterial(fileName);
	//surfaces[surfaceId].material = material;
}

//
// PolymerNGModelCache::SetMiscForModelSurface
//
void PolymerNGModelCache::SetMiscForModelSurface(int32_t modelid, float scale, int32_t shadeoff, float zadd, float yoffset, int32_t flags)
{
	surfaceOverrideDefines[modelid].scale = scale;
	surfaceOverrideDefines[modelid].shadeoff = shadeoff;
	surfaceOverrideDefines[modelid].zadd = zadd;
	surfaceOverrideDefines[modelid].yoffset = yoffset;
	surfaceOverrideDefines[modelid].flags = flags;
}

//
// PolymerNGModelCache::SetModelTile(
//
bool PolymerNGModelCache::SetModelTile(const char *fileName, int tileNum)
{
	PolymerNGModelCachePayload *currentPayload = NULL;
	ModelCachePayloadHeader *currentPayloadInfo = NULL;

	if (numPayloads == -1)
	{
		return false;
	}

	for (int i = 0; i < numPayloads; i++)
	{
		if (!stricmp(payloadHeaders[i].modelpath, fileName))
		{
			currentPayloadInfo = &payloadHeaders[i];
			currentPayload = &payloads[i];
			break;
		}
	}

	if (currentPayloadInfo == NULL || currentPayload == NULL)
		return false;

	tileCacheOverride[tileNum].payload = currentPayload;
	tileCacheOverride[tileNum].payloadHeader = currentPayloadInfo;

	return true;
}