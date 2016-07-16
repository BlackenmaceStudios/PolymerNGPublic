#pragma once

#include <stdint.h>
#include "../../DukeNukem/Build/include/compat.h"

#define SHIFTMOD32(a) ((a)&31)

#define VOXBORDWIDTH 1 //use 0 to save memory, but has texture artifacts; 1 looks better...
#define VOXUSECHAR 0

#if (VOXUSECHAR != 0)
typedef struct { uint8_t x, y, z, u, v; } vert_t;
#else
typedef struct { uint16_t x, y, z, u, v; } vert_t;
#endif

typedef struct { vert_t v[4]; } voxrect_t;

typedef struct
{
	//WARNING: This top block is a union of md2model,md3model,voxmodel: Make sure it matches!
	int32_t mdnum; //VOX=1, MD2=2, MD3=3. NOTE: must be first in structure!
	int32_t shadeoff;
	float scale, bscale, zadd;
	uint32_t *texid;    // skins for palettes
	int32_t flags;

	//VOX specific stuff:
	voxrect_t *quad; int32_t qcnt, qfacind[7];
	int32_t *mytex, mytexx, mytexy;
	vec3_t siz;
	vec3f_t piv;
	int32_t is8bit;
} voxmodel_t;

void voxfree(voxmodel_t *m);
voxmodel_t *voxload(const char *filnam);