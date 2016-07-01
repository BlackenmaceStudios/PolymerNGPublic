// TextureCache.cpp
//

#include "../PolymerNG_local.h"
#include "../Renderer/Renderer.h"
#include "../../engine_priv.h"
#include "TextureCache.h"

#include "../../../../Third-Party/zlib/zlib.h"

PolymerNGTextureCache::PolymerNGTextureCache()
{
	LoadTextureCache();
	memset(&tileCacheOverride, 0, sizeof(PolymerNGTileCacheOverride) * MAXTILES);
}

void PolymerNGTextureCache::LoadTextureCache()
{
	PayloadHeader header;
	cacheFile = BuildFile::OpenFile(TEXTURECACHE_FILENAME, BuildFile::BuildFile_Read);
	if (cacheFile == NULL)
	{
		numPayloads = -1;
		initprintf("Failed to load texture cache.\n");
		isLoaded = false;
		return;
	}

	// Read in the texture cache header.
	cacheFile->Read(&header, sizeof(PayloadHeader));
	payloadInfo = new CachePayloadInfo[header.numPayloads];
	payloads = new PolymerNGTextureCachePayload[header.numPayloads];

	numPayloads = header.numPayloads;

	// Read in all the payload info.
	cacheFile->Read(payloadInfo, header.numPayloads * sizeof(CachePayloadInfo));

	isLoaded = true;
	initprintf("Texture Cache has %d payloads\n", numPayloads);
	// We never close the cache file!
	//delete cacheFile;
}

void PolymerNGTextureCache::BeginLevelLoad()
{
	if (numPayloads == -1)
		return;

	totalSizeOfHighQualityAssets = 0;
	// Free all previous loaded payloads.
	for (int i = 0; i < numPayloads; i++)
	{
		if (payloads[i].payloadbuffer)
		{
			delete payloads[i].payloadbuffer;
			payloads[i].payloadbuffer = NULL;
		}
	}
}

void PolymerNGTextureCache::EndLevelLoad()
{
	if (numPayloads == -1)
		return;

	initprintf("--------PolymerNGTextureCache::EndLevelLoad---------\n");
	initprintf("..%dmb of high resolution textures\n", totalSizeOfHighQualityAssets >> 20);
	initprintf("----------------------------------------------------\n");
}

const PolymerNGTextureCacheResidentData *PolymerNGTextureCache::LoadHighqualityTextureForTile(int tileNum, PolymerNGTextureCachePayloadImageType payloadImageType)
{
	PolymerNGTextureCachePayload *currentPayload = tileCacheOverride[payloadImageType][tileNum].payload;
	CachePayloadInfo *currentPayloadInfo = tileCacheOverride[payloadImageType][tileNum].payloadInfo;

	if (residentDataStorage[payloadImageType][tileNum].rawImageDataBlob)
		return &residentDataStorage[payloadImageType][tileNum];

	if (tileCacheOverride[payloadImageType][tileNum].payload == NULL)
		return NULL;

	if (currentPayload == NULL || currentPayloadInfo == NULL)
		return NULL;

	return ReadDataFromTextureCache(currentPayload, currentPayloadInfo, &residentDataStorage[payloadImageType][tileNum]);

	return NULL;
}

const PolymerNGTextureCacheResidentData *PolymerNGTextureCache::LoadHighqualityTexture(const char *name)
{
	PolymerNGTextureCachePayload *currentPayload = NULL;
	CachePayloadInfo *currentPayloadInfo = NULL;

	if (numPayloads == -1)
	{
		return false;
	}

	std::size_t name_hash = name_hash = std::hash<std::string>()(name);

	// Check to see if its already loaded.
	for (int i = 0; i < residentDataStorageDyanmic.size(); i++)
	{
		if (name_hash == residentDataStorageDyanmic[i].name_hash)
			return &residentDataStorageDyanmic[i];
	}

	// Check to see if this texture is in the texture cache.
	for (int i = 0; i < numPayloads; i++)
	{
		if (!strcmp(payloadInfo[i].cacheFileName, name))
		{
			currentPayloadInfo = &payloadInfo[i];
			break;
		}
	}

	// Bail if its not in the cache.
	if (currentPayloadInfo == NULL)
		return NULL;

	PolymerNGTextureCacheResidentData newResidentDataBlank;
	residentDataStorageDyanmic.push_back(newResidentDataBlank);
	
	return ReadDataFromTextureCache(NULL, currentPayloadInfo, &residentDataStorageDyanmic[residentDataStorageDyanmic.size() - 1]);
}

const PolymerNGTextureCacheResidentData *PolymerNGTextureCache::ReadDataFromTextureCache(PolymerNGTextureCachePayload *currentPayload, CachePayloadInfo *currentPayloadInfo, PolymerNGTextureCacheResidentData *storage)
{
	byte *compressedBuffer = new byte[currentPayloadInfo->compressedPayloadLength];
	byte *decompressedBuffer = new byte[currentPayloadInfo->decompressedPayloadLength];
	cacheFile->Seek(currentPayloadInfo->startPosition, SEEK_SET);
	cacheFile->Read(compressedBuffer, currentPayloadInfo->compressedPayloadLength);

	z_stream infstream;
	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;
	// setup "b" as the input and "c" as the compressed output
	infstream.avail_in = (uInt)(currentPayloadInfo->compressedPayloadLength); // size of input
	infstream.next_in = (Bytef *)compressedBuffer; // input char array
	infstream.avail_out = (uInt)currentPayloadInfo->decompressedPayloadLength; // size of output
	infstream.next_out = (Bytef *)decompressedBuffer; // output char array
													  // the actual DE-compression work.
	inflateInit(&infstream);
	inflate(&infstream, Z_NO_FLUSH);
	inflateEnd(&infstream);

	//BuildImage *image = polymerNG.AllocHighresImage(tileNum, currentPayloadInfo->width, currentPayloadInfo->height, decompressedBuffer);
	storage->width = currentPayloadInfo->width;
	storage->height = currentPayloadInfo->height;
	storage->format = currentPayloadInfo->format;
	storage->rawImageDataBlob = decompressedBuffer;
	std::string name = currentPayloadInfo->cacheFileName;
	storage->name_hash = std::hash<std::string>()(name);
	delete compressedBuffer;
	//delete decompressedBuffer;

	//	currentPayload->payloadbuffer = NULL;
	totalSizeOfHighQualityAssets += payloadInfo->decompressedPayloadLength;

	return storage;
}

bool PolymerNGTextureCache::SetHighQualityTextureForTile(const char *fileName, int tileNum, PolymerNGTextureCachePayloadImageType payloadImageType)
{
	PolymerNGTextureCachePayload *currentPayload = NULL;
	CachePayloadInfo *currentPayloadInfo = NULL;

	if (numPayloads == -1)
	{
		return false;
	}

	for (int i = 0; i < numPayloads; i++)
	{
		if (!strcmp(payloadInfo[i].cacheFileName, fileName))
		{
			currentPayloadInfo = &payloadInfo[i];
			currentPayload = &payloads[i];
			break;
		}
	}

	if (currentPayloadInfo == NULL || currentPayload == NULL)
		return false;

	tileCacheOverride[payloadImageType][tileNum].payload = currentPayload;
	tileCacheOverride[payloadImageType][tileNum].payloadInfo = currentPayloadInfo;

	return true;
}