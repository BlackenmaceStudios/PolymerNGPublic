// PolymerNG_SoftwareRasterizer.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

#include "../engine_priv.h"
#include "Models/Models.h"

extern std::vector<int32_t> pvsVisibleSectors;

/*
=====================
PolymerNGSoftwarRasterizer::PolymerNGSoftwarRasterizer
=====================
*/
PolymerNGSoftwarRasterizer::PolymerNGSoftwarRasterizer(int width, int height)
{
	this->width = width;
	this->height = height;

	this->rasterizationBuffer = new byte[width * height * 4];
	this->visibleSectorList = new int32_t[width * height];
}

/*
=====================
PolymerNGSoftwarRasterizer::GetVisibleSectorList
=====================
*/
int32_t *PolymerNGSoftwarRasterizer::GetVisibleSectorList()
{
	if (pvsVisibleSectors.size() == 0)
		return NULL;

	return &pvsVisibleSectors[0];
}

/*
=====================
PolymerNGSoftwarRasterizer::GetNumVisibleSectors
=====================
*/
int PolymerNGSoftwarRasterizer::GetNumVisibleSectors()
{
	return pvsVisibleSectors.size();
}

/*
=====================
PolymerNGSoftwarRasterizer::IsSectorVisible
=====================
*/
bool PolymerNGSoftwarRasterizer::IsSectorVisible(int32_t sectorNum)
{
	//for (int i = 0; i < width * height; i++)
	//{
	//	if (visibleSectorList[i] == sectorNum)
	//	{
	//		return true;
	//	}
	//}
	if (std::find(pvsVisibleSectors.begin(), pvsVisibleSectors.end(), sectorNum) != pvsVisibleSectors.end()) {
		// someName not in name, add it
		return true;
	}
	return false;
}

/*
=====================
PolymerNGSoftwarRasterizer::RenderOcclusion
=====================
*/
void PolymerNGSoftwarRasterizer::RenderOcclusion(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum)
{
	static int oldxres = xres;
	static int oldyres = yres;
	isOcclusionPass = true;
	bytesperline = width;
	xres = width;
	yres = height;

	frameplace = (intptr_t)&rasterizationBuffer[0];

	pvsVisibleSectors.clear();

	setgamemode(0, width, height, 32);
	drawrooms(daposx, daposy, daposz, daang, dahoriz, dacursectnum);
	memcpy(visibleSectorList, rasterizationBuffer, width * height * sizeof(int32_t));
	setgamemode(0, 1024, 768, 32);

	xres = oldxres;
	yres = oldyres;
	bytesperline = 0;
	isOcclusionPass = false;
}