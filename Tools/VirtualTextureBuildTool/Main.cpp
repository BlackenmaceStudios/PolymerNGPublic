// Main.cpp
//

#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <stack>
#include <direct.h>
#include <algorithm>

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
		printf("Making Payload %s with stats (%dx%d)\n", files[i].c_str(), imageinfo.Width, imageinfo.Height);

		CachePayloadInternal payload;
		payload.compressedDataBlob = new byte[imageinfo.Width * imageinfo.Height];
		payload.zlibDataBlob = new byte[imageinfo.Width * imageinfo.Height]; // Allocate the zlib data blob to be the same size as the dxt5 data blob.
		if (!ilGetDXTCData(payload.compressedDataBlob, imageinfo.Width * imageinfo.Height, IL_DXT3))
		{
			printf("ilImageToDxtcData: Failed to convert image\n");
			return 0;
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
		printf("Compressing Payload (%d/%d)\n", i, files.size());
		defstream.avail_in = payloads[i].info.width * payloads[i].info.height; // size of input, string + terminator
		defstream.next_in = (Bytef *)payloads[i].compressedDataBlob; // input char array
		defstream.avail_out = (uInt)payloads[i].info.width * payloads[i].info.height; // size of output
		defstream.next_out = (Bytef *)payloads[i].zlibDataBlob; // output char array

		deflateInit(&defstream, Z_BEST_COMPRESSION);
		deflate(&defstream, Z_FINISH);
		deflateEnd(&defstream);

		payloads[i].info.startPosition = ftell(cacheFile);
		payloads[i].info.compressedPayloadLength = defstream.total_out;
		payloads[i].info.decompressedPayloadLength = payloads[i].info.width * payloads[i].info.height;
		fwrite(payloads[i].zlibDataBlob, defstream.total_out, 1, cacheFile);
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