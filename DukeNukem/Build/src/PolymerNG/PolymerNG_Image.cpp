// PolymerNG_Image.cpp
//

#include "PolymerNG_local.h"

BuildImage::BuildImage(BuildImageOpts imageOpts)
{
	texture = NULL;
	this->imageOpts = imageOpts;

	if (imageOpts.tileNum == -1)
	{
		name = imageOpts.name;
	}
	else
	{
		name = stringFormat(L"buildImage_%d", imageOpts.tileNum);
	}
}

//
// BuildImage::IsLoaded
//
bool BuildImage::IsLoaded()
{
	return texture != NULL;
}

//
// BuildImage::GetWidth
//
int BuildImage::GetWidth()
{
	if (imageOpts.width != -1)
		return imageOpts.width;

	return tilesiz[imageOpts.tileNum].x;
}

//
// BuildImage::GetHeight
//
int BuildImage::GetHeight()
{
	if (imageOpts.height != -1)
		return imageOpts.height;

	return tilesiz[imageOpts.tileNum].y;
}

//
// GetDepth
//
int BuildImage::GetDepth()
{
	return imageOpts.depth;
}

//
// BuildImage::ConvertARTImage
//
char *BuildImage::ConvertARTImage()
{
	char *tilebuffer = (char *)waloff[imageOpts.tileNum];
	char *tempbuffer = (char *)Xmalloc(tilesiz[imageOpts.tileNum].x * tilesiz[imageOpts.tileNum].y);
	int i, j, k;

	i = k = 0;
	while (i < tilesiz[imageOpts.tileNum].y) {
		j = 0;
		while (j < tilesiz[imageOpts.tileNum].x) {
			tempbuffer[k] = tilebuffer[(j * tilesiz[imageOpts.tileNum].y) + i];
			k++;
			j++;
		}
		i++;
	}

	return tempbuffer;
}

//
// BuildImage::LoadInArtData
//
void BuildImage::LoadInArtData()
{
	// Ensure this image is loaded, do the hd loads in the game thread.
	if (!waloff[imageOpts.tileNum])
		loadtile(imageOpts.tileNum);
}

//
// BuildImage::UpdateImagePost
//
void BuildImage::UpdateImagePost(byte *buffer)
{
	char *tempbuffer = NULL;
	
	if (imageOpts.inputBuffer)
	{
		tempbuffer = (char *)imageOpts.inputBuffer;
	}
	else if (buffer)
	{
		tempbuffer = (char *)buffer;
	}
	else if(imageOpts.tileNum != -1)
	{
		tempbuffer = ConvertARTImage();
		imageOpts.width = tilesiz[imageOpts.tileNum].x;
		imageOpts.height = tilesiz[imageOpts.tileNum].y;
	}

	if (!IsLoaded())
	{
		bool allowCPUWrites = (imageOpts.heapType == BUILDIMAGE_ALLOW_CPUWRITES);

		BuildRHITextureFormat format = BuildRHI::GetRHITextureFormat(imageOpts.format);

		switch (imageOpts.imageType)
		{
		case IMAGETYPE_1D:
			texture = BuildRHI::LoadTextureFromMemory(name, GetWidth(), 0, format, (const void *)tempbuffer, allowCPUWrites, imageOpts.allowCPUReads, imageOpts.isRenderTargetImage);
			break;
		case IMAGETYPE_2D:
			texture = BuildRHI::LoadTextureFromMemory(name, GetWidth(), GetHeight(), format, (const void *)tempbuffer, allowCPUWrites, imageOpts.allowCPUReads, imageOpts.isRenderTargetImage);
			break;
		case IMAGETYPE_CUBE:
			texture = BuildRHI::LoadTextureCubeFromMemory(name, GetWidth(), GetHeight(), format, (const void *)tempbuffer, allowCPUWrites, imageOpts.allowCPUReads, imageOpts.isRenderTargetImage);
			imageOpts.imageType = IMAGETYPE_2D; // THIS IS A HACK!!
			break;

	//	case IMAGETYPE_3D:
	//		texture = BuildRHI::LoadTexture3DFromMemory(name, GetWidth(), GetHeight(), GetDepth(), format, (const void *)tempbuffer, allowCPUWrites);
	//		break;
		}

//		texture->SetHardwareDebugName(name.c_str());

		if (!buffer && !imageOpts.inputBuffer)
		{
			Bfree(tempbuffer);
		}
		return;
	}

	if (imageOpts.heapType != BUILDIMAGE_ALLOW_CPUWRITES)
	{
		initprintf("BuildImage::UpdateImage: Image not set for CPU Writes");
		if (!buffer)
		{
			Bfree(tempbuffer);
		}
		return;
	}

	texture->UploadRegion(0, 0, GetWidth(), GetHeight(), (const void *)tempbuffer);
	if (!buffer)
	{
		Bfree(tempbuffer);
	}
}