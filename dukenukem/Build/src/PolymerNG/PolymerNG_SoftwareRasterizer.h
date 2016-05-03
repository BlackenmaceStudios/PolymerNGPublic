// PolymerNG_SoftwarRasterizer.h
//

#pragma once

extern bool isOcclusionPass;

//
// PolymerNGSoftwarRasterizer
//
class PolymerNGSoftwarRasterizer
{
public:
	PolymerNGSoftwarRasterizer(int width, int height);

	void RenderOcclusion(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum);

	bool IsSectorVisible(int32_t sectorNum);

	int32_t *GetVisibleSectorList();
	int GetNumVisibleSectors();
private:
	int width;
	int height;
	byte *rasterizationBuffer;

	int32_t *visibleSectorList;
};
