// Main.cpp
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include "voxmodel.h"

//
// WriteTGATexture
//
bool WriteTGATexture(const char *filename, byte *picbuf, int32_t width, int32_t height, int32_t is8bit, int32_t dapal)
{
	byte	*buffer;
	int		c;

	buffer = (byte *)malloc(width*height * 4 + 18);
	memset(buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 32;	// pixel size

						// swap rgb to bgr
	c = 18 + width * height * 4;
	for (int i = 18, d = 0; i < c; i += 4, d+=3)
	{
		buffer[i] = picbuf[d + 3];		// blue
		buffer[i + 1] = picbuf[d + 3];		// green
		buffer[i + 2] = picbuf[d + 3];		// red
		buffer[i + 3] = 255;		// alpha
	}

	// This is stupi and not clean - but it works.
	char tgaFileName[4096];
	strcpy(tgaFileName, filename);
	tgaFileName[strlen(tgaFileName) - 3] = 't';
	tgaFileName[strlen(tgaFileName) - 2] = 'g';
	tgaFileName[strlen(tgaFileName) - 1] = 'a';

	FILE *file = fopen(tgaFileName, "wb");
	fwrite(buffer, c, 1, file);
	fclose(file);

	return true;
}

//
// Main
//
int main(int argc, char **argv)
{
	char *cwd = _getcwd(NULL, 0);

	printf("Voxel2obj v0.01 by Justin Marshall\n");
	char *srcFile = argv[1];

	printf("Loading %s\n", srcFile);
	voxmodel_t *voxelModel = voxload(srcFile);
	if (!voxelModel)
	{
		printf("Failed to load model\n");
		return 1;
	}

	char destFile[4096];
	srcFile[strlen(srcFile) - 4] = 0;
	sprintf(destFile, "%s.obj", srcFile);
	printf("Writing %s\n", destFile);

	FILE *objModel = fopen(destFile, "w+");
	if (!objModel)
	{
		printf("Failed to open dest file\n");
		return 1;
	}

	const float phack[2] = { 0, 1.f / 256.f };

	for (int32_t i = 0, fi = 0; i < voxelModel->qcnt; i++)
	{
		const vert_t *const vptr = &voxelModel->quad[i].v[0];

		const int32_t xx = vptr[0].x + vptr[2].x;
		const int32_t yy = vptr[0].y + vptr[2].y;
		const int32_t zz = vptr[0].z + vptr[2].z;

		for (int32_t j = 0; j < 4; j++)
		{
//#if (VOXBORDWIDTH == 0)
//			bglTexCoord2f(((float)vptr[j].u)*ru + uhack[vptr[j].u != vptr[0].u],
//				((float)vptr[j].v)*rv + vhack[vptr[j].v != vptr[0].v]);
//#else
//			bglTexCoord2f(((float)vptr[j].u)*ru, ((float)vptr[j].v)*rv);
//#endif
			float x = ((float)vptr[j].x) - phack[xx > vptr[j].x * 2] + phack[xx < vptr[j].x * 2];
			float y = ((float)vptr[j].y) - phack[yy > vptr[j].y * 2] + phack[yy < vptr[j].y * 2];
			float z = ((float)vptr[j].z) - phack[zz > vptr[j].z * 2] + phack[zz < vptr[j].z * 2];


			fprintf(objModel, "v %f %f %f\n", x, y, z);
		}
	}

	fprintf(objModel, "\n");

	const float ru = 1.f / ((float)voxelModel->mytexx);
	const float rv = 1.f / ((float)voxelModel->mytexy);

	for (int32_t i = 0, fi = 0; i < voxelModel->qcnt; i++)
	{
		const vert_t *const vptr = &voxelModel->quad[i].v[0];

		for (int32_t j = 0; j < 4; j++)
		{
			fprintf(objModel, "vt %f %f\n", ((float)vptr[j].u)*ru, ((float)vptr[j].v)*rv);
		}
	}

	for (int i = 0, d = 1; i < voxelModel->qcnt; i++,d+=4)
	{
		fprintf(objModel, "f %d/%d %d/%d %d/%d %d/%d\n", d+3, d+3, d+2, d+2, d+1, d+1, d+0, d+0);
	}

	fclose(objModel);

	printf("Writing texture.\n");
	WriteTGATexture(destFile, (byte *)voxelModel->mytex, voxelModel->mytexx, voxelModel->mytexy, voxelModel->is8bit, NULL);

	return 0;
}