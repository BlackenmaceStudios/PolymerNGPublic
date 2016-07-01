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
	fontImage = NULL;
	memset(images, 0, sizeof(BuildImage *) * MAXTILES);
}

//
// PolymerNGPaletteManager::AllocFontImage
//
BuildImage *PolymerNGImageManager::AllocFontImage(const char *smallFont, const char *bigFont)
{
	// construct a 256x128 8-bit alpha-only texture for the font glyph matrix
	char * const tbuf = (char *)Xmalloc(256 * 128);

	Bmemset(tbuf, 0, 256 * 128);

	char * cptr = (char *)bigFont;

	for (int h = 0; h < 256; h++)
	{
		char *tptr = tbuf + (h % 32) * 8 + (h / 32) * 256 * 8;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (cptr[h * 8 + i] & pow2char[7 - j]) tptr[j] = 255;
			}
			tptr += 256;
		}
	}

	cptr = (char *)smallFont;

	for (int h = 0; h < 256; h++)
	{
		char *tptr = tbuf + 256 * 64 + (h % 32) * 8 + (h / 32) * 256 * 8;
		for (int i = 1; i < 7; i++)
		{
			for (int j = 2; j < 6; j++)
			{
				if (cptr[h * 8 + i] & pow2char[7 - j]) tptr[j - 2] = 255;
			}
			tptr += 256;
		}
	}

	if (fontImage == NULL)
	{
		BuildImageOpts opts;
		opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;
		opts.imageType = IMAGETYPE_2D;
		opts.width = (256);
		opts.height = 128;
		opts.format = IMAGE_FORMAT_R8;
		opts.name = L"FontImage";
		opts.tileNum = -1;
		opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;
		fontImage = new BuildImage(opts);
	}

	fontImage->UpdateImagePost((byte *)tbuf);
	return fontImage;
}


//
// PolymerNGImageManager::SetHighQualityTextureForTile
//
bool PolymerNGImageManager::SetHighQualityTextureForTile(const char *fileName, int tileNum, PolymerNGTextureCachePayloadImageType payloadImageType)
{
	if (textureCache == NULL)
		return false;

	if (!textureCache->IsLoaded())
		return false;

	return textureCache->SetHighQualityTextureForTile(fileName, tileNum, payloadImageType);
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
		opts.format = IMAGE_FORMAT_RGBA8;
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

	byte *blackBlankTextureBuffer = new byte[2 * 2 * 4];
	blackImage = AllocHighresImage(-1, 2, 2, blackBlankTextureBuffer, IMAGE_FORMAT_RGBA8);
	blackImage->UpdateImagePost(NULL);
	delete blackBlankTextureBuffer;
}

//
// PolymerNG::AllocHighresImage
//
BuildImage *PolymerNGImageManager::AllocHighresImage(int idx, int width, int height, byte *buffer, BuildImageFormat format)
{
	BuildImageOpts opts;
	opts.width = width;
	opts.height = height;
	opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;
	opts.depth = 1;
	opts.tileNum = idx;
	opts.format = format;
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
		BuildImageFormat format = (BuildImageFormat)-1;
		switch (data->format)
		{
		case TEXTURE_CACHE_UNCOMPRESSED:
			format = IMAGE_FORMAT_RGBA8;
			break;
		case TEXTURE_CACHE_DXT1:
			format = IMAGE_FORMAT_DXT1;
			break;
		case TEXTURE_CACHE_DXT3:
			format = IMAGE_FORMAT_DXT3;
			break;
		case TEXTURE_CACHE_DXT5:
			format = IMAGE_FORMAT_DXT5;
			break;
		case TEXTURE_CACHE_BC3:
			format = IMAGE_FORMAT_3DC;
			break;
		}
		image = AllocHighresImage(-1, data->width, data->height, data->rawImageDataBlob, format);
		AppendImageToUploadQueue(image);
	}

	return image;
}

//
// PolymerNGImageManager::LoadFromTileId
//
BuildImage *PolymerNGImageManager::LoadFromTileId(int tilenum, PolymerNGTextureCachePayloadImageType payloadImageType)
{
	if (images[tilenum][payloadImageType] != NULL)
		return images[tilenum][payloadImageType];

	// If we have a texture cache, load the hi resolution texture from there.
	if (textureCache->IsLoaded())
	{
		const PolymerNGTextureCacheResidentData *data = textureCache->LoadHighqualityTextureForTile(tilenum, payloadImageType);

		if (data)
		{
			BuildImageFormat format = (BuildImageFormat)-1;
			switch (data->format)
			{
			case TEXTURE_CACHE_UNCOMPRESSED:
				format = IMAGE_FORMAT_RGBA8;
				break;
			case TEXTURE_CACHE_DXT1:
				format = IMAGE_FORMAT_DXT1;
				break;
			case TEXTURE_CACHE_DXT3:
				format = IMAGE_FORMAT_DXT3;
				break;
			case TEXTURE_CACHE_DXT5:
				format = IMAGE_FORMAT_DXT5;
				break;
			case TEXTURE_CACHE_BC3:
				format = IMAGE_FORMAT_DXT5;
				break;
			case TEXTURE_CACHE_RXGB:
				format = IMAGE_FORMAT_DXT5;
				break;
			}

			images[tilenum][payloadImageType] = AllocHighresImage(tilenum, data->width, data->height, data->rawImageDataBlob, format);
		}	
	}

	// We only have the old data, so load in the original tile data.
	if(images[tilenum][payloadImageType] == NULL)
	{
		if (payloadImageType == PAYLOAD_IMAGE_DIFFUSE)
		{
			// Load in the original texture data
			images[tilenum][payloadImageType] = AllocArtTileImage(tilenum);
			images[tilenum][payloadImageType]->LoadInArtData();
		}
		else
		{
			return NULL;
		}
	}

	AppendImageToUploadQueue(images[tilenum][payloadImageType]);
	return images[tilenum][payloadImageType];
}

//
// PolymerNGImageManager::FlushTile
//
void PolymerNGImageManager::FlushTile(int16_t tileNum)
{
	if (images[tileNum][PAYLOAD_IMAGE_DIFFUSE] == NULL)
	{
		LoadFromTileId(tileNum, PAYLOAD_IMAGE_DIFFUSE);
	}

	images[tileNum][PAYLOAD_IMAGE_DIFFUSE]->LoadInArtData();
}


//
// PolymerNGImageManager::LoadImage
//
//BuildImage *PolymerNGImageManager::LoadImage(const char *name, bool hires)
//{
//
//}