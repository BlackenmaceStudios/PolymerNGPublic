// Main.cpp
//

#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <stack>
#include <direct.h>
#include <algorithm>
#include <assert.h>

#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"

#include "../../DukeNukem/Third-Party/zlib/zlib.h"
#include "../../DukeNukem/Build/src/PolymerNG/TextureCache/TextureCacheFormat.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

using namespace std;

bool ListFiles(string path, string mask, vector<string>& files) {
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA ffd;
	string spec;
	stack<string> directories;

	directories.push(path);
	files.clear();

	while (!directories.empty()) {
		path = directories.top();
		spec = path + "\\" + mask;
		directories.pop();

		hFind = FindFirstFileA(spec.c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE) {
			return false;
		}
		do {
			if (strcmp(ffd.cFileName, ".") != 0 &&
				strcmp(ffd.cFileName, "..") != 0) {
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					directories.push(path + "\\" + ffd.cFileName);
				}
				else 
				{
					if (strstr(ffd.cFileName, ".png") || strstr(ffd.cFileName, ".jpg"))
					{
						files.push_back(path + "\\" + ffd.cFileName);
					}
				}
			}
		} while (FindNextFileA(hFind, &ffd) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES) {
			FindClose(hFind);
			return false;
		}

		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}

	return true;
}

bool is_power_of_2(int i) 
{
	if (i <= 0) {
		return 0;
	}
	return !(i & (i - 1));
}

//
// Main
//
int main(int argc, char **argv)
{
	vector<string> files;

	char *cwd = _getcwd(NULL, 0);

	printf("VirtualTextureBuilder v0.01 by Justin Marshall\n");
	printf("Finding PNG files...\n");
	if (!ListFiles(cwd, "*", files))
	{
		return 0;
	}

	printf("Found %d assets\n", files.size());

	ilInit();

	std::vector<CachePayloadInternal> payloads;
	payloads.reserve(files.size());

	int numNonPowerOfTwoImages = 0;
	// Loop through all the images
	for (int i = 0; i < files.size(); i++)
	{
		if (!ilLoadImage(files[i].c_str()))
		{
			printf("ilLoadImage: Failed to load %s\n", files[i].c_str());
			return 0;
		}

		ILinfo imageinfo;
		iluGetImageInfo(&imageinfo);
		if (!is_power_of_2(imageinfo.Width) || !is_power_of_2(imageinfo.Height))
		{
			int scaled_width, scaled_height;
			for (scaled_width = 1; scaled_width < imageinfo.Width; scaled_width <<= 1)
			{

			}
			for (scaled_height = 1; scaled_height < imageinfo.Height; scaled_height <<= 1)
			{

			}
			printf("Making Payload %s with stats RESIZED (%dx%d) from (%dx%d)\n", files[i].c_str(), scaled_width, scaled_height, imageinfo.Width, imageinfo.Height);

			iluScale(scaled_width, scaled_height, ilGetInteger(IL_IMAGE_DEPTH));

			iluGetImageInfo(&imageinfo);
		}
		else
		{
			printf("Making Payload %s with stats (%dx%d)\n", files[i].c_str(), imageinfo.Width, imageinfo.Height);
		}

		CachePayloadInternal payload;
		ILenum compressionFormat = IL_DXT5;
		payload.info.format = TEXTURE_CACHE_DXT5;

		// HACK _n.png/jpg and _s.png/jpg
		if (strstr(files[i].c_str(), "_n.png") || strstr(files[i].c_str(), "_n.jpg"))
		{
			if (is_power_of_2(imageinfo.Width) && is_power_of_2(imageinfo.Height))
			{
				compressionFormat = IL_RXGB;
				payload.info.format = TEXTURE_CACHE_BC3;
				printf("...Using BC3 compression\n");
			}
			else
			{
				//char temp[1024];
				//sprintf(temp, "Detected NON POT - %d (%s - %dx%d)\n", numNonPowerOfTwoImages, files[i].c_str(), imageinfo.Width, imageinfo.Height);
				//OutputDebugStringA(temp);
				//payload.info.format = TEXTURE_CACHE_UNCOMPRESSED;
				//numNonPowerOfTwoImages++;
				//printf("...Using TEXTURE_CACHE_UNCOMPRESSED because non power of 2\n");
				assert(0);
			}
		}
		else if (strstr(files[i].c_str(), "_s.png") || strstr(files[i].c_str(), "_s.jpg"))
		{
			compressionFormat = IL_DXT1;
			payload.info.format = TEXTURE_CACHE_DXT1;
			printf("...Using DXT1 compression\n");
		}
		else
		{
			if (is_power_of_2(imageinfo.Width) && is_power_of_2(imageinfo.Height))
			{
				// I hate DevIL, I can't figure how to know if there is a alpha channel or not when the PNG was compressed with using
				// there awful PNG index system. Oh well, just force indexed palettes to have a alpha for the time being. :/.
				if (imageinfo.Format == IL_RGBA || imageinfo.Format == IL_COLOUR_INDEX)
				{
					compressionFormat = IL_DXT5;
					payload.info.format = TEXTURE_CACHE_DXT5;
					printf("...Using DXT5 compression(alpha channel detected)\n");
				}
				else
				{
					compressionFormat = IL_DXT1;
					payload.info.format = TEXTURE_CACHE_DXT1;
					printf("...Using DXT1 compression(no alpha channel detected)\n");
				}
			}
			else
			{
				//char temp[1024];
				//sprintf(temp, "Detected NON POT - %d (%s - %dx%d)\n", numNonPowerOfTwoImages, files[i].c_str(), imageinfo.Width, imageinfo.Height);
				//OutputDebugStringA(temp);
				//payload.info.format = TEXTURE_CACHE_UNCOMPRESSED;
				//numNonPowerOfTwoImages++;
				//printf("...Using TEXTURE_CACHE_UNCOMPRESSED because non power of 2\n");
				assert(0);
			}
		}

		payload.info.decompressedPayloadLength = imageinfo.SizeOfData;

		if (payload.info.format == TEXTURE_CACHE_UNCOMPRESSED)
		{
			payload.info.decompressedPayloadLength = imageinfo.Width * imageinfo.Height * 4;
			payload.compressedDataBlob = new byte[payload.info.decompressedPayloadLength];
			payload.zlibDataBlob = new byte[payload.info.decompressedPayloadLength]; // Allocate the zlib data blob to be the same size as the dxt5 data blob.

			byte *palette = ilGetPalette();

			if (palette == NULL)
			{
				ilConvertImage(IL_RGBA, IL_BYTE);
				iluGetImageInfo(&imageinfo);
				memcpy(payload.compressedDataBlob, imageinfo.Data, payload.info.decompressedPayloadLength);
			}
			else
			{
				for (int d = 0, f = 0; d < imageinfo.Width * imageinfo.Height; d++, f+=4)
				{
					byte *pal = &palette[imageinfo.Data[d]];
					payload.compressedDataBlob[f + 0] = pal[0];
					payload.compressedDataBlob[f + 1] = pal[1];
					payload.compressedDataBlob[f + 2] = pal[2];
					payload.compressedDataBlob[f + 3] = 255;
				}
			}
		}
		else
		{
			payload.compressedDataBlob = new byte[payload.info.decompressedPayloadLength];
			payload.zlibDataBlob = new byte[payload.info.decompressedPayloadLength]; // Allocate the zlib data blob to be the same size as the dxt5 data blob.
			if (!ilGetDXTCData(payload.compressedDataBlob, payload.info.decompressedPayloadLength, compressionFormat))
			{
				printf("ilImageToDxtcData: Failed to convert image\n");
				return 0;
			}
		}
		std::replace(files[i].begin(), files[i].end(), '\\', '/');
		strcpy(payload.info.cacheFileName, files[i].c_str() + strlen(cwd) + 1);
		payload.info.width = imageinfo.Width;
		payload.info.height = imageinfo.Height;
		payloads.push_back(payload);
	}

	printf("Writing game payload file...\n");

	// Write out our payload info's.
	FILE *cacheFile = fopen("game_textures.payloads", "wb");
	PayloadHeader header;
	header.numPayloads = files.size();
	fwrite(&header, sizeof(PayloadHeader), 1, cacheFile);

	int payloadStartPosition = ftell(cacheFile);
	for (int i = 0; i < files.size(); i++)
	{
		fwrite(&payloads[i].info, sizeof(CachePayloadInfo), 1, cacheFile);
	}

	// zlib struct
	z_stream defstream;
	defstream.zalloc = Z_NULL;
	defstream.zfree = Z_NULL;
	defstream.opaque = Z_NULL;

	// Now write out all the payloads, while recording were each payload is in the file.
	for (int i = 0; i < files.size(); i++)
	{
		printf("Compressing Payload (%d/%d)", i, files.size());

		defstream.avail_in = payloads[i].info.decompressedPayloadLength; // size of input, string + terminator
		defstream.next_in = (Bytef *)payloads[i].compressedDataBlob; // input char array
		defstream.avail_out = payloads[i].info.decompressedPayloadLength; // size of output
		defstream.next_out = (Bytef *)payloads[i].zlibDataBlob; // output char array

		deflateInit(&defstream, Z_BEST_COMPRESSION);
		deflate(&defstream, Z_FINISH);
		deflateEnd(&defstream);

		payloads[i].info.startPosition = ftell(cacheFile);
		payloads[i].info.compressedPayloadLength = defstream.total_out;
		fwrite(payloads[i].zlibDataBlob, defstream.total_out, 1, cacheFile);		
		printf(" %d bytes zlib compressed to %d bytes\n", payloads[i].info.decompressedPayloadLength, payloads[i].info.compressedPayloadLength);
	}

	// Re-write the header with the payload info.
	fseek(cacheFile, payloadStartPosition, SEEK_SET);
	for (int i = 0; i < files.size(); i++)
	{
		fwrite(&payloads[i].info, sizeof(CachePayloadInfo), 1, cacheFile);
	}

	fclose(cacheFile);
	return 0;
}