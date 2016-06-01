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

	bool hasShadows;
};

//
// PolymerNGLight
//
class PolymerNGLight
{
public:

};

//
// PolymerNGPublic
//
class PolymerNGPublic
{
public:
	virtual PolymerNGLight *AddLightToCurrentBoard(PolymerNGLightOpts lightOpts) = 0;
};

extern PolymerNGPublic *polymerNGPublic;