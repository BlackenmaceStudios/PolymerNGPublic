// PolymerNG_ImageManager.h
//

#pragma once

//
// PolymerNGPaletteManager
//
class PolymerNGPaletteManager
{
public:
	void UpdatePalette(int idx);
	void UpdatePaletteLookupTable(int idx);

	BuildImage *GetPaletteImage();
	BuildImage *GetPaletteImage(int idx) { return palette_image[idx]; }
	BuildImage *GetPaletteLookupImage(int idx) { return paletteLookUp_image[idx]; }
private:
	BuildImage *palette_image[256];
	BuildImage *paletteLookUp_image[256];
};

//
// PolymerNGImageManager
//
class PolymerNGImageManager
{
public:
	PolymerNGImageManager();
	void	Init();

	void BeginLevelLoad();
	void EndLevelLoad();

	BuildImage *LoadFromTileId(int tilenum, PolymerNGTextureCachePayloadImageType payloadImageType);
	BuildImage *LoadTexture(const char *name);

	void FlushTile(int16_t tileNum);

	// Upload pending image data, this should be called on the render thread only!
	void UploadPendingImages();

	// Sets the high quality texture for tile.
	bool SetHighQualityTextureForTile(const char *fileName, int tileNum, PolymerNGTextureCachePayloadImageType payloadImageType);

	// Returns the handle to the palette manager.
	PolymerNGPaletteManager *GetPaletteManager() { return &paletteManager; }

	// Allocates the font image.
	BuildImage *AllocFontImage(const char *smallFont, const char *bigFont);

	// Returns a black image.
	BuildImage *GetBlackImage() { return blackImage; }
private:
	BuildImage *AllocArtTileImage(int idx);
	BuildImage *AllocHighresImage(int idx, int width, int height, byte *buffer, BuildImageFormat format);

	void AppendImageToUploadQueue(BuildImage *image);

	BuildImage *images[MAXTILES][PAYLOAD_IMAGE_NUMTYPES];

	PolymerNGPaletteManager paletteManager;
	PolymerNGTextureCache *textureCache;

	BuildImage *fontImage;

	BuildImage *blackImage;

	int numImagesWaitingForUpload;
	BuildImage *images_waiting_for_upload[MAX_QUEUED_IMAGES * 2];
};

extern PolymerNGImageManager imageManager;