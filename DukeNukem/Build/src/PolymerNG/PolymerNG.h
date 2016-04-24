// PolymerNG.h
//

#pragma once

#define MAX_QUEUED_IMAGES	MAXTILES
#define POLYMER_INLINE		__forceinline

#include <vector>

#include "../RHI/BuildRHI.h"
#include "PolymerNG_Image.h"
#include "../../Include/build3d.h"
#include "Models/Models.h"


//
// PolymerNG
//
class PolymerNG
{
public:
	void	    Init();

	// Upadtes the palette.
	void		UpdatePalette(int idx = 0);

	// Updates the hardware image.
	void		FlushTile(int16_t tileNum);

	// Uploads pending images.
	void		UploadPendingImages();

	// Adds a image to the update queue.
	void		AddImageToUpdateQueue(BuildImage *image);

	// Loads in all classic textures into memory.
	void		LoadAllClassicTextures();

	// Loads in the new board.
	void		LoadBoard();
	void		DrawRooms(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum);

	BuildImage *GetImage(int texnum) { return images[texnum]; }

	BuildImage *GetPaletteImage();
	BuildImage *GetPaletteImage(int idx) { return palette_image[idx]; }
private:
	BuildImage *palette_image[256];

	BuildImage *images[MAXTILES];

	int numImagesWaitingForUpload;
	BuildImage *images_waiting_for_upload[MAX_QUEUED_IMAGES];
};

extern PolymerNG polymerNG;