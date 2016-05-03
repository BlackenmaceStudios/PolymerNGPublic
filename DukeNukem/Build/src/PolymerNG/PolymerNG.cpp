// PolymerNG.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"
#include "../engine_priv.h"

PolymerNG polymerNG;
PolymerNGPrivate polymerNGPrivate;

// THIS IS A HACK - RIPPED FROM DUKE3D.h
#define TILE_SAVESHOT       (MAXTILES-1)
#define TILE_LOADSHOT       (MAXTILES-3)
#define TILE_ANIM           (MAXTILES-4)

//
// PolymerNG
//
void PolymerNG::Init()
{
	initprintf("------ PolymerNG::Init --------\n");

	// Init the RHI.
	rhi.Init();
	
	// Init the image pool.
	for (int i = 0; i < MAXTILES; i++)
	{
		BuildImageOpts opts;

		opts.tileNum = i;
		opts.heapType = BUILDIMAGE_CPU_HEAP_NONE;

		if (i == TILE_SAVESHOT || i == TILE_LOADSHOT || i == TILE_ANIM)
			opts.heapType = BUILDIMAGE_ALLOW_CPUWRITES;

		images[i] = new BuildImage(opts);
	}

	numImagesWaitingForUpload = 0;
	renderer.Init();
}

//
// GetPaletteImage
//
BuildImage *PolymerNG::GetPaletteImage() 
{ 
//	initprintf("GetPaletteImage: Returning %d\n", curbasepal);
	return palette_image[curbasepal]; 
}
//
// UpdatePaletteLookupTable
//
void PolymerNG::UpdatePaletteLookupTable(int idx)
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

//
// UpdatePalette
//
void PolymerNG::UpdatePalette(int idx)
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

//
// AddImageToUpdateQueue
//
void PolymerNG::AddImageToUpdateQueue(BuildImage *image)
{
//	for (int i = 0; i < numImagesWaitingForUpload; i++)
//	{
//		if (image->GetOpts().tileNum != -1 && images_waiting_for_upload[i]->GetOpts().tileNum == image->GetOpts().tileNum)
//		{
//			return;
//		}
//	}
	images_waiting_for_upload[numImagesWaitingForUpload++] = image;
}

//
// PolymerNG::UploadPendingImages
//
void PolymerNG::UploadPendingImages()
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
// LoadAllClassicTextures
//
void PolymerNG::LoadAllClassicTextures()
{
	for (int i = 0; i < MAXTILES; i++)
	{
		if (images[i]->GetWidth() > 2 && images[i]->GetHeight() > 2)
			images[i]->UpdateImage();
	}
}

//
// PolymerNG::FlushTile
//
void PolymerNG::FlushTile(int16_t tileNum)
{
	BuildImage *image = images[tileNum];

	image->UpdateImage();
}