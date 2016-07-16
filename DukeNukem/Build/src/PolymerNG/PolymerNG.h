// PolymerNG.h
//

#pragma once

#define MAX_QUEUED_IMAGES	MAXTILES
#define POLYMER_INLINE		__forceinline

#define POLYMERNG_NOSYNC_SPRITES		1

#include <vector>

#include "../RHI/BuildRHI.h"
#include "PolymerNG_Image.h"
#include "PolymerNG_Material.h"
#include "../../Include/build3d.h"
#include "Models/Models.h"
#include "PolymerNG_Material.h"
#include "TextureCache/TextureCache.h"
#include "PolymerNG_public.h"

//
// PolymerNG
//
class PolymerNG : public PolymerNGPublic
{
public:
	void	    Init();

	// Upadtes the palette.
	void		UpdatePalette(int idx = 0);

	// Updates the palette looked up table.
	void		UpdatePaletteLookupTable(int idx = 0);

	// Updates the hardware image.
	void		FlushTile(int16_t tileNum);

	// Uploads pending images.
	void		UploadPendingImages();

	PolymerNGLight *AddLightToCurrentBoard(PolymerNGLightOpts lightOpts);
	virtual void RemoveLightFromCurrentBoard(PolymerNGLight *);

	// Loads in the new board.
	void		LoadBoard();
	void		DrawRooms(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum);


	bool		SetHighQualityTextureForTile(const char *fileName, int tileNum, PolymerNGTextureCachePayloadImageType payloadImageType);

	// Allocates the font image.
	PolymerNGMaterial	*AllocFontImage(const char *smallFont, const char *bigFont);

	virtual void		MoveLightsInSector(int sectorNum, float deltax, float deltay);

	virtual void SetAmbientLightForSector(int sectorNum, int ambientLightNum);
//	BuildImage *GetImage(int texnum) { return images[texnum]; }
//
//	BuildImage *GetHighresImage(int idx) { return hiresImages[idx]; }
//	BuildImage *AllocHighresImage(int idx, int width, int height, byte *buffer);
private:
};

extern PolymerNG polymerNG;