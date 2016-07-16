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
bool PolymerNG::SetHighQualityTextureForTile(const char *fileName, int tileNum, PolymerNGTextureCachePayloadImageType payloadImageType)
{
	return imageManager.SetHighQualityTextureForTile(fileName, tileNum, payloadImageType);
}

//
// PolymerNG::UpdatePalette
//
void PolymerNG::UpdatePalette(int idx)
{
	imageManager.GetPaletteManager()->UpdatePalette(idx);
}

//
// PolymerNG::AllocFontImage
//
PolymerNGMaterial *PolymerNG::AllocFontImage(const char *smallFont, const char *bigFont)
{
	BuildImage *image = imageManager.AllocFontImage(smallFont, bigFont);
	PolymerNGMaterial *material = materialManager.LoadMaterialFromImage("FontImage", image);
	return material;
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

//
// PolymerNG::MoveLightsInSector
//
void PolymerNG::MoveLightsInSector(int sectorNum, float deltax, float deltay)
{
	polymerNGPrivate.currentBoard->MoveLightsInSector(sectorNum, deltax, deltay);
}

//
// PolymerNG::SetAmbientLightForSector
//
void PolymerNG::SetAmbientLightForSector(int sectorNum, int ambientLightNum)
{
	polymerNGPrivate.currentBoard->SetAmbientLightForSector(sectorNum, ambientLightNum);
}