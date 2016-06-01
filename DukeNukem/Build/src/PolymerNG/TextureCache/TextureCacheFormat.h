// TextureCacheFormat.h
//

#pragma once

#include <stdio.h>

#define PAYLOAD_IDEN "jmpayload"

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