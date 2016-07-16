// ModelCache.h
//

#pragma once

#ifdef SWGAME
	#define MODELCACHE_FILENAME "Assets\\SWData\\game_meshes.payloads"
#else
	#define MODELCACHE_FILENAME "Assets\\DukeData\\game_meshes.payloads"
#endif

class CacheModel;
class BuildFile;

#define MAX_MODEL_SURFACES 20

//
// PolymerNGModelCachePayload
//
struct PolymerNGModelCachePayload
{
	PolymerNGModelCachePayload()
	{
		memset(this, 0, sizeof(PolymerNGModelCachePayload));
	}

	ModelCachePayloadInfo	modelCacheInfo;
	ModelCacheSurface		surfaces[MAX_MODEL_SURFACES];

	unsigned int	*indexes;
	Build3DVertex *vertexes;
};

//
// PolymerNGModelCacheOverride
//
struct PolymerNGModelCacheOverride
{
	PolymerNGModelCachePayload *payload;
	ModelCachePayloadHeader *payloadHeader;
};

//
// PolymerNGModelCache
//
class PolymerNGModelCache
{
public:
	PolymerNGModelCache();

	void BeginLevelLoad();
	void EndLevelLoad(BaseModel *model);

	PolymerNGModelCacheSurfaceDefine *GetModelOverridesForId(int idx) {
		return &surfaceOverrideDefines[idx];
	}

	void DefineTextureForModelSurface(int modelId, int surfaceId, const char *fileName);
	void SetMiscForModelSurface(int32_t modelid, float scale, int32_t shadeoff, float zadd, float yoffset, int32_t flags);
	CacheModel *LoadModelForTile(int tileNum);
	bool SetModelTile(const char *fileName, int tileNum);
private:
	void LoadModelCache();

	BuildFile *cacheFile;
	int numPayloads;
	int totalSizeOfHighQualityAssets;

	ModelCachePayloadHeader *payloadHeaders;
	PolymerNGModelCachePayload *payloads;

	PolymerNGModelCacheSurfaceDefine surfaceOverrideDefines[MAXTILES];
	CacheModel *loadedmodels[MAXTILES];
	PolymerNGModelCacheOverride tileCacheOverride[MAXTILES];
};

extern PolymerNGModelCache modelCacheSystem;