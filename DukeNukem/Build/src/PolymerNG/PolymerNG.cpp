// PolymerNG.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"
#include "../engine_priv.h"

PolymerNG polymerNG;
PolymerNGPrivate polymerNGPrivate;
PolymerNGPublic *polymerNGPublic = &polymerNG;

//
// PolymerNG
//
void PolymerNG::Init()
{
	initprintf("------ PolymerNG::Init --------\n");

	// Init the RHI.
	rhi.Init();

	renderer.Init();
	imageManager.Init();
}

//
// PolymerNG::UploadPendingImages
//
void PolymerNG::UploadPendingImages()
{
	imageManager.UploadPendingImages();
}

//
// PolymerNG::SetHighQualityTextureForTile
//
bool PolymerNG::SetHighQualityTextureForTile(const char *fileName, int tileNum)
{
	return imageManager.SetHighQualityTextureForTile(fileName, tileNum);
}

//
// PolymerNG::UpdatePalette
//
void PolymerNG::UpdatePalette(int idx)
{
	imageManager.GetPaletteManager()->UpdatePalette(idx);
}

//
// PolymerNG::UpdatePaletteLookupTable
//
void PolymerNG::UpdatePaletteLookupTable(int idx)
{
	imageManager.GetPaletteManager()->UpdatePaletteLookupTable(idx);
}

//
// PolymerNG::FlushTile
//
void PolymerNG::FlushTile(int16_t tileNum)
{
	imageManager.FlushTile(tileNum);
}