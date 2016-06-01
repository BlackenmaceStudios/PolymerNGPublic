// PolymerNG_ImageManager.cpp
//

#include "PolymerNG_local.h"
#include "../engine_priv.h"

PolymerNGImageManager imageManager;

// THIS IS A HACK - RIPPED FROM DUKE3D.h
#define TILE_SAVESHOT       (MAXTILES-1)
#define TILE_LOADSHOT       (MAXTILES-3)
#define TILE_ANIM           (MAXTILES-4)

//
// PolymerNGPaletteManager::GetPaletteImage
//
BuildImage *PolymerNGPaletteManager::GetPaletteImage()
{
	//	initprintf("GetPaletteImage: Returning %d\n", curbasepal);
	return palette_image[curbasepal];
}
//
// PolymerNGPaletteManager::UpdatePaletteLookupTable
//
void PolymerNGPaletteManager::UpdatePaletteLookupTable(int idx)
{
	// We need to add a alpha channel.
	const int expectedNumShades = 32;

	byte tempPaletteBuffer[256 * expectedNumShades];

	if (paletteLookUp_image[idx] != NULL || palookup[idx] == NULL)
		return;

	wchar_t palette_name[512];
	swprintf(palette_name, L"build_numshadespalette%d", idx);

	// Load in system textures.
	{
		BuildImageOpts opts;
		opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;
		opts.imageType = IMAGETYPE_2D;
		opts.width = (256);
		opts.height = expectedNumShades;
		opts.format = IMAGE_FORMAT_R8;
		opts.name = palette_name;
		opts.tileNum = -1;
		opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;
		paletteLookUp_image[idx] = new BuildImage(opts);
	}

	for (int i = 0; i < 256 * expectedNumShades; i++)
	{
		tempPaletteBuffer[i] = palookup[idx][i];
	}
	paletteLookUp_image[idx]->UpdateImagePost(tempPaletteBuffer);


}

PolymerNGImageManager::PolymerNGImageManager()
{
	numImagesWaitingForUpload = 0;
	memset(images, 0, sizeof(BuildImage *) * MAXTILES);
}

//
// PolymerNGImageManager::SetHighQualityTextureForTile
//
bool PolymerNGImageManager::SetHighQualityTextureForTile(const char *fileName, int tileNum)
{
	if (textureCache == NULL)
		return false;

	if (!textureCache->IsLoaded())
		return false;

	return textureCache->SetHighQualityTextureForTile(fileName, tileNum);
}

//
// PolymerNG::UploadPendingImages
//
void PolymerNGImageManager::UploadPendingImages()
{
	for (int i = 0; i < numImagesWaitingForUpload; i++)
	{
		if (!images_waiting_for_upload[i]->IsLoaded() || images_waiting_for_upload[i]->AllowsCPUWrites())
		{
			images_waiting_for_upload[i]->UpdateImagePost(NULL);
		}
	}

	numImagesWaitingForUpload = 0;
}

//
// PolymerNGPaletteManager::UpdatePalette
//
void PolymerNGPaletteManager::UpdatePalette(int idx)
{
	// We need to add a alpha channel.
	byte tempPaletteBuffer[256 * 4];

	if (palette_image[idx] != NULL)
		return;

	wchar_t palette_name[512];
	swprintf(palette_name, L"build_palette%d", idx);

	// Load in system textures.
	{
		BuildImageOpts opts;
		opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;
		opts.imageType = IMAGETYPE_1D;
		opts.width = sizeof(palette) / 3;
		opts.height = 0;
		opts.format = IMAGE_FORMAT_RGBA32;
		opts.name = palette_name;
		opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;
		opts.tileNum = -1;
		palette_image[idx] = new BuildImage(opts);
	}

	for (int i = 0; i < 256; i++)
	{
		tempPaletteBuffer[(i * 4) + 0] = basepaltable[idx][(i * 3) + 0];
		tempPaletteBuffer[(i * 4) + 1] = basepaltable[idx][(i * 3) + 1];
		tempPaletteBuffer[(i * 4) + 2] = basepaltable[idx][(i * 3) + 2];
		tempPaletteBuffer[(i * 4) + 3] = 255;
	}
	palette_image[idx]->UpdateImagePost(tempPaletteBuffer);

	if (idx == 0)
	{
		UpdatePaletteLookupTable(idx);
	}
}


void PolymerNGImageManager::Init()
{
	initprintf("----------PolymerNGImageManager::Init--------\n");
	textureCache = new PolymerNGTextureCache();
}

//
// PolymerNG::AllocHighresImage
//
BuildImage *PolymerNGImageManager::AllocHighresImage(int idx, int width, int height, byte *buffer)
{
	BuildImageOpts opts;
	opts.width = width;
	opts.height = height;
	opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;
	opts.depth = 1;
	opts.tileNum = idx;
	opts.format = IMAGE_FORMAT_DXT5;
	opts.inputBuffer = buffer;
	opts.isHighQualityImage = true;

	return new BuildImage(opts);
}

//
// PolymerNGImageManager::AllocArtTileImage
//
BuildImage *PolymerNGImageManager::AllocArtTileImage(int idx)
{
	BuildImageOpts opts;

	opts.tileNum = idx;
	opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;

	if (idx  == TILE_SAVESHOT || idx == TILE_LOADSHOT || idx == TILE_ANIM)
		opts.heapType = BUILDIMAGE_ALLOW_CPUWRITES;

	return new BuildImage(opts);
}

//
// PolymerNGImageManager::BeginLevelLoad
//
void PolymerNGImageManager::BeginLevelLoad()
{
	textureCache->BeginLevelLoad();
}

//
// PolymerNGImageManager::EndLevelLoad
//
void PolymerNGImageManager::EndLevelLoad()
{
	textureCache->EndLevelLoad();
}


//
// PolymerNGImageManager::AppendImageToUploadQueue
//
void PolymerNGImageManager::AppendImageToUploadQueue(BuildImage *image)
{
	// !!Todo make this thread safe!!!
	images_waiting_for_upload[numImagesWaitingForUpload++] = image;
}

//
// PolymerNGImageManager::LoadTexture
//
BuildImage *PolymerNGImageManager::LoadTexture(const char *name)
{
	// We can only non art textures from the cache, these are always high res.
	if (!textureCache->IsLoaded())
		return NULL;

	const PolymerNGTextureCacheResidentData *data = textureCache->LoadHighqualityTexture(name);

	BuildImage *image = NULL;
	if (data)
	{
		image = AllocHighresImage(-1, data->width, data->height, data->rawImageDataBlob);
		AppendImageToUploadQueue(image);
	}

	return image;
}

//
// PolymerNGImageManager::LoadFromTileId
//
BuildImage *PolymerNGImageManager::LoadFromTileId(int tilenum)
{
	if (images[tilenum] != NULL)
		return images[tilenum];

	// If we have a texture cache, load the hi resolution texture from there.
	if (textureCache->IsLoaded())
	{
		const PolymerNGTextureCacheResidentData *data = textureCache->LoadHighqualityTextureForTile(tilenum);

		if (data)
		{
			images[tilenum] = AllocHighresImage(tilenum, data->width, data->height, data->rawImageDataBlob);
		}	
	}

	// We only have the old data, so load in the original tile data.
	if(images[tilenum] == NULL)
	{
		// Load in the original texture data
		images[tilenum] = AllocArtTileImage(tilenum);
		images[tilenum]->LoadInArtData();
	}

	AppendImageToUploadQueue(images[tilenum]);
	return images[tilenum];
}

//
// PolymerNGImageManager::FlushTile
//
void PolymerNGImageManager::FlushTile(int16_t tileNum)
{
	if (images[tileNum] == NULL)
	{
		LoadFromTileId(tileNum);
	}

	images[tileNum]->LoadInArtData();
}


//
// PolymerNGImageManager::LoadImage
//
//BuildImage *PolymerNGImageManager::LoadImage(const char *name, bool hires)
//{
//
//}