// PolymerNG_public.h
//

#pragma once

/*
=====================================

Public code that gets exposed to the game module.

=====================================
*/

//
// PolymerNGLightType
//
enum PolymerNGLightType
{
	POLYMERNG_LIGHTTYPE_POINT = 0,
	POLYMERNG_LIGHTTYPE_SPOT,
	POLYMERNG_LIGHTTYPE_PARALLEL,
	POLYMERNG_LIGHTTYPE_AREA
};

//
// PolymerNGLightOpts
//
struct PolymerNGLightOpts
{
	PolymerNGLightOpts()
	{
		memset(this, 0, sizeof(PolymerNGLightOpts));
	}
	PolymerNGLightType lightType;
	float	position[4];
	float   color[4];

	int sector;
	int range;
	int radius;
	int faderaiud;
	int angle;
	int horiz;
	int minshade;
	int maxshade;
	int priority;
	int tilenum;
	int faderadius;
	int brightness;

	bool castShadows;
};

//
// PolymerNGLight
//
class PolymerNGLight
{
public:
	virtual PolymerNGLightOpts *GetOpts() = 0;
	virtual const PolymerNGLightOpts *GetOriginalOpts() = 0;
};

//
// PolymerNGPublic
//
class PolymerNGPublic
{
public:
	virtual PolymerNGLight *AddLightToCurrentBoard(PolymerNGLightOpts lightOpts) = 0;

	virtual void MoveLightsInSector(int sectorNum, float deltax, float deltay) = 0;
};

extern PolymerNGPublic *polymerNGPublic;