// TextureCache.h
//

#pragma once

#include "TextureCacheFormat.h"

#define TEXTURECACHE_FILENAME "Assets\\DukeData\\game_textures.payloads"

class BuildFile;

//
// PolymerNGTextureCachePayload
//
struct PolymerNGTextureCachePayload
{
	PolymerNGTextureCachePayload()
	{
		payloadbuffer = NULL;
	}
	byte *payloadbuffer;
};

//
// PolymerNGTileCacheOverride
//
struct PolymerNGTileCacheOverride
{
	PolymerNGTextureCachePayload *payload;
	CachePayloadInfo *payloadInfo;
};

//
// PolymerNGTextureCacheResidentData
//
struct PolymerNGTextureCacheResidentData
{
	PolymerNGTextureCacheResidentData()
	{
		rawImageDataBlob = NULL;
		width = -1;
		height = -1;
		name_hash = -1;
	}
	byte *rawImageDataBlob;
	int width;
	int height;
	std::size_t name_hash;
};

//
// PolymerNGTextureCache
//
class PolymerNGTextureCache
{
public:
	PolymerNGTextureCache();

	void BeginLevelLoad();
	void EndLevelLoad();

	bool IsLoaded() { return isLoaded; }

	const PolymerNGTextureCacheResidentData *LoadHighqualityTextureForTile(int tileNum);
	const PolymerNGTextureCacheResidentData *LoadHighqualityTexture(const char *name);
	bool SetHighQualityTextureForTile(const char *fileName, int tileNum);
private:
	void LoadTextureCache();

	const PolymerNGTextureCacheResidentData *ReadDataFromTextureCache(PolymerNGTextureCachePayload *currentPayload, CachePayloadInfo *currentPayloadInfo, PolymerNGTextureCacheResidentData *storage);

	CachePayloadInfo *payloadInfo;
	PolymerNGTextureCachePayload *payloads;
	BuildFile *cacheFile;

	int totalSizeOfHighQualityAssets;
	int numPayloads;

	bool isLoaded;

	PolymerNGTileCacheOverride tileCacheOverride[MAXTILES];
	PolymerNGTextureCacheResidentData residentDataStorage[MAXTILES];
	std::vector<PolymerNGTextureCacheResidentData> residentDataStorageDyanmic;
};
