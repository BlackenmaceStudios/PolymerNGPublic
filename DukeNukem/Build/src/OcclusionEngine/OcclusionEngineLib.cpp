#include "stdafx.h"
#include "OcclusionEngineLib.h"
#include "OcclusionEngine.h"

OcclusionHandle InitializeOcclusionEngineDefault(int width, int height)
{
	OcclusionEngineOptions options;
	options.drawAllTiles = false; //Do not draw covered tiles by default.
	options.engineMode = Optimized;
	options.numberOfThreads = 0; //Automatically assigned.
	options.tileSize = 16;		//Default Tile Size;
	return new OcclusionEngine(width, height, options);
}

OcclusionHandle InitializeOcclusionEngine(int width, int height, OcclusionEngineOptions options)
{
	return new OcclusionEngine(width, height, options);
}

bool  addOccluders(OcclusionHandle handle, const OccluderData occludersData[], const int numberOfOccluders) 
 {
	 return handle->addOccluders(occludersData, numberOfOccluders);
 }

bool testOccludeeVisibility(OcclusionHandle handle,const OccludeeData occludee)
 {
	 return handle->testOccludeeVisibility(occludee);
 }

float getDepthBufferPixel(OcclusionHandle handle,const int x, const int y)
 {
	 return handle->getDepthBufferPixel(x, y);
 }

void clearOcclusionEngine(OcclusionHandle handle)
{
	return handle->clear();
}

void disposeOcclusionEngine(OcclusionHandle handle)
{
	return handle->dispose();
}
