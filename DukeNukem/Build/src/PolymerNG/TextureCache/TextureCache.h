// TextureCache.h
//

#pragma once

#include "TextureCacheFormat.h"

#ifdef SWGAME
#define TEXTURECACHE_FILENAME "Assets\\SWData\\game_textures.payloads"
#else
#define TEXTURECACHE_FILENAME "Assets\\DukeData\\game_textures.payloads"
#endif

class BuildFile;

//
// PolymerNGTextureCachePayloadImageType
//
enum PolymerNGTextureCachePayloadImageType
{
	PAYLOAD_IMAGE_DIFFUSE = 0,
	PAYLOAD_IMAGE_NORMAL,
	PAYLOAD_IMAGE_SPECULAR,
	PAYLOAD_IMAGE_GLOW,
	PAYLOAD_IMAGE_NUMTYPES
};

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
		format = (TextureCacheImageFormat)-1;
	}
	byte *rawImageDataBlob;
	TextureCacheImageFormat format;
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

	const PolymerNGTextureCacheResidentData *LoadHighqualityTextureForTile(int tileNum, PolymerNGTextureCachePayloadImageType payloadImageType);
	const PolymerNGTextureCacheResidentData *LoadHighqualityTexture(const char *name);
	bool SetHighQualityTextureForTile(const char *fileName, int tileNum, PolymerNGTextureCachePayloadImageType payloadImageType);
private:
	void LoadTextureCache();

	const PolymerNGTextureCacheResidentData *ReadDataFromTextureCache(PolymerNGTextureCachePayload *currentPayload, CachePayloadInfo *currentPayloadInfo, PolymerNGTextureCacheResidentData *storage);

	CachePayloadInfo *payloadInfo;
	PolymerNGTextureCachePayload *payloads;
	BuildFile *cacheFile;

	int totalSizeOfHighQualityAssets;
	int numPayloads;

	bool isLoaded;

	PolymerNGTileCacheOverride tileCacheOverride[PAYLOAD_IMAGE_NUMTYPES][MAXTILES];
	PolymerNGTextureCacheResidentData residentDataStorage[PAYLOAD_IMAGE_NUMTYPES][MAXTILES];
	std::vector<PolymerNGTextureCacheResidentData> residentDataStorageDyanmic;
};
