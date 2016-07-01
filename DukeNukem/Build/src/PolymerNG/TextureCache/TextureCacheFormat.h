// TextureCacheFormat.h
//

#pragma once

#include <stdio.h>

#define PAYLOAD_IDEN "jmpayload"

//
// TextureCacheImageFormat
//
enum TextureCacheImageFormat
{
	TEXTURE_CACHE_UNCOMPRESSED,
	TEXTURE_CACHE_DXT1,
	TEXTURE_CACHE_DXT3,
	TEXTURE_CACHE_DXT5,
	TEXTURE_CACHE_BC3,
	TEXTURE_CACHE_DXTC,
	TEXTURE_CACHE_RXGB
};

//
// CachePayloadInfo
//
struct CachePayloadInfo
{
	CachePayloadInfo()
	{
		memset(cacheFileName, 0, sizeof(cacheFileName));
	}
	char cacheFileName[128];
	TextureCacheImageFormat format;
	int width;
	int height;
	int startPosition;
	int compressedPayloadLength;
	int decompressedPayloadLength;
};

//
// PayloadHeader
//
struct PayloadHeader
{
	PayloadHeader()
	{
		strcpy(iden, PAYLOAD_IDEN);
		numPayloads = 0;
	}
	char iden[9];
	int numPayloads;
};


//
// CachePayload
//
struct CachePayloadInternal
{
	CachePayloadInfo info;
	byte *compressedDataBlob;
	byte *zlibDataBlob;
};