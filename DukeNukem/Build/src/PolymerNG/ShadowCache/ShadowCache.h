// ShadowCache.h
//

#pragma once

#define NUM_QUEUED_SHADOW_MAPS 10

//
// ShadowCacheEntry
//
struct ShadowCacheEntry
{
	ShadowCacheEntry()
	{
		light = NULL;
	}

	PolymerNGLight *light;
	bool isDirty;
};

//
// ShadowCache
//
class ShadowCache
{
public:

private:

};