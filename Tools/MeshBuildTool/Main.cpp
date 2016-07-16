// Main.cpp
//

#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <stack>
#include <direct.h>
#include <algorithm>

#include "AssetImport/include/assimp/Importer.hpp"
#include "AssetImport/include/assimp/scene.h"
#include "AssetImport/include/assimp/postprocess.h"

#define NOASM

#include "../../DukeNukem/Build/include/build3d.h"
#include "../../DukeNukem/Build/src/PolymerNG/Models/ModelCacheFormat.h"

#include "../../DukeNukem/Third-Party/zlib/zlib.h"

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
					if (strstr(ffd.cFileName, ".md3") || strstr(ffd.cFileName, ".md2") || strstr(ffd.cFileName, ".fbx") || strstr(ffd.cFileName, ".3ds") || strstr(ffd.cFileName, ".ase") || strstr(ffd.cFileName, ".blender") || strstr(ffd.cFileName, ".obj"))
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
// CovertedMeshInternal
//
struct CovertedMeshInternal
{
	ModelCachePayloadInfo meshCacheInfo;
	std::vector<ModelCacheSurface> surfaces;
	std::vector<Build3DVertex> vertexes;
	std::vector<unsigned int> indexes;
};

std::vector<CovertedMeshInternal> meshes;
std::vector<ModelCachePayloadHeader> payloadHeaders;

void WriteCompressedData(FILE *file, void *data, int data_length)
{
	// zlib struct
	z_stream defstream;
	defstream.zalloc = Z_NULL;
	defstream.zfree = Z_NULL;
	defstream.opaque = Z_NULL;

	byte *compressed_buffer = new byte[data_length];

	// Now write out all the payloads, while recording were each payload is in the file.
	defstream.avail_in = data_length; // size of input, string + terminator
	defstream.next_in = (Bytef *)data; // input char array
	defstream.avail_out = data_length; // size of output
	defstream.next_out = (Bytef *)compressed_buffer; // output char array

	deflateInit(&defstream, Z_BEST_COMPRESSION);
	deflate(&defstream, Z_FINISH);
	deflateEnd(&defstream);

	fwrite(compressed_buffer, defstream.total_out, 1, file);

	delete compressed_buffer;
}

//
// Main
//
int main(int argc, char **argv)
{
	vector<string> files;

	char *cwd = _getcwd(NULL, 0);

	printf("MeshBuildTool v0.01 by Justin Marshall\n");
	printf("Finding model files...\n");
	if (!ListFiles(cwd, "*", files))
	{
		return 0;
	}

	printf("Found %d assets\n", files.size());

	for (int i = 0; i < files.size(); i++)
	{
		printf("Processing(%d/%d) %s\n", i, files.size(), files[i].c_str());
		OutputDebugStringA(files[i].c_str());
		OutputDebugStringA("\n");

		Assimp::Importer Importer;

		const aiScene* pScene = Importer.ReadFile(files[i].c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
		if (pScene == NULL)
		{
			printf("...Failed to load!\n");
			OutputDebugStringA("...Failed to load\n");
			return 0;
		}

		printf("...%d mesh detected\n", pScene->mNumMeshes);

		CovertedMeshInternal mesh;
		mesh.surfaces.resize(pScene->mNumMeshes);

		// Create all the meshes.
		for (int d = 0; d < pScene->mNumMeshes; d++)
		{
			aiMesh *_aimesh = pScene->mMeshes[d];
			ModelCacheSurface *_cacheMesh = &mesh.surfaces[d];

			_cacheMesh->startVertex = mesh.vertexes.size();
			_cacheMesh->numVertexes = _aimesh->mNumVertices;

			// Grab all the vertexes.
			for (int v = 0; v < _aimesh->mNumVertices; v++)
			{
				Build3DVertex vertex;

				vertex.position.x = _aimesh->mVertices[v].x;
				vertex.position.y = _aimesh->mVertices[v].y;
				vertex.position.z = _aimesh->mVertices[v].z;

				vertex.uv.x = _aimesh->mTextureCoords[0][v].x;
				vertex.uv.y = _aimesh->mTextureCoords[0][v].y;

				mesh.vertexes.push_back(vertex);
			}

			_cacheMesh->startIndex = mesh.indexes.size();
			_cacheMesh->numIndexes = 0;

			// Grab the surface name which is the mesh name(todo we need to do some preprocessing here probably).
			strcpy(_cacheMesh->material, _aimesh->mName.C_Str());

			// Grab all the indexes.
			for (int v = 0; v < _aimesh->mNumFaces; v++)
			{
				aiFace *face = &_aimesh->mFaces[v];
				for (int ii = 0; ii < face->mNumIndices; ii++)
				{
					mesh.indexes.push_back(face->mIndices[ii]);
					_cacheMesh->numIndexes++;
				}
			}
		}

		ModelCachePayloadHeader payloadHeader;
		memset(payloadHeader.modelpath, 0, sizeof(payloadHeader.modelpath));
		std::replace(files[i].begin(), files[i].end(), '\\', '/');
		strcpy(payloadHeader.modelpath, files[i].c_str() + strlen(cwd) + 1);
		payloadHeader.modelWritePosition = 0;
		payloadHeaders.push_back(payloadHeader);

		mesh.meshCacheInfo.numFrames = 1;
		mesh.meshCacheInfo.numVertexes = mesh.vertexes.size();
		mesh.meshCacheInfo.numIndexes = mesh.indexes.size();
		mesh.meshCacheInfo.numSurfaces = mesh.surfaces.size();

		meshes.push_back(mesh);
	}

	printf("Writing mesh payloads files.\n");
	// Write out our payload info's.
	FILE *cacheFile = fopen("game_meshes.payloads", "wb");
	ModelCacheHeader header;
	header.numPayloads = meshes.size();
	fwrite(&header, sizeof(ModelCacheHeader), 1, cacheFile);

	// Write out all the payload headers.
	int payloadStartPosition = ftell(cacheFile);
	for (int i = 0; i < meshes.size(); i++)
	{
		fwrite(&payloadHeaders[i], sizeof(ModelCachePayloadHeader), 1, cacheFile);
	}

	// Write out each model.
	for (int i = 0; i < meshes.size(); i++)
	{
		CovertedMeshInternal *mesh = &meshes[i];

		printf("Writing Mesh Data(%d/%d)\n", i, meshes.size());

		payloadHeaders[i].modelWritePosition = ftell(cacheFile);

		// Write out the cache info.
		fwrite(&mesh->meshCacheInfo, sizeof(ModelCachePayloadInfo), 1, cacheFile);
		
		// Write out each surface for this mesh.
		for (int d = 0; d < mesh->surfaces.size(); d++)
		{
			fwrite(&mesh->surfaces[d], sizeof(ModelCacheSurface), 1, cacheFile);
		}

		// Write out all the vertexes.
		fwrite(&mesh->vertexes[0], sizeof(Build3DVertex) * mesh->vertexes.size(), 1, cacheFile);
		//WriteCompressedData(cacheFile, &mesh->vertexes[0], sizeof(Build3DVertex) * mesh->vertexes.size());

		// Write out all the indexes.
		fwrite(&mesh->indexes[0], sizeof(unsigned int) * mesh->indexes.size(), 1, cacheFile);
		//WriteCompressedData(cacheFile, &mesh->indexes[0], sizeof(unsigned int) * mesh->indexes.size());
	}

	// Re-write out all the payload headers with the updated position.
	fseek(cacheFile, payloadStartPosition, SEEK_SET);
	for (int i = 0; i < meshes.size(); i++)
	{
		fwrite(&payloadHeaders[i], sizeof(ModelCachePayloadHeader), 1, cacheFile);
	}
	return 0;
};