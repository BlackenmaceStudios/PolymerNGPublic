//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2005 - 3D Realms Entertainment

This file is NOT part of Shadow Warrior version 1.2
However, it is either an older version of a file that is, or is
some test code written during the development of Shadow Warrior.
This file is provided purely for educational interest.

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#include "pch.h"
#include "build.h"
#include "compat.h"
#include "../Editor/editor.h"

#include "keys.h"
#include "names2.h"
#include "game.h"


extern long posx, posy, posz;
extern short cursectnum;
extern short ang;

BOOL FindCeilingView(short match, LONGp x, LONGp y, LONG z, SHORTp sectnum);
BOOL FindFloorView(short match, LONGp x, LONGp y, LONG z, SHORTp sectnum);
short ViewSectorInScene(short cursectnum, short type, short level);
void Message(char *string, char color);


////////////////////////////////////////////////////////////////////
//
// FLOOR ABOVE FLOOR
//
////////////////////////////////////////////////////////////////////


#define ZMAX 400
typedef struct
{
	LONG zval[ZMAX];
	SHORT sectnum[ZMAX];
	SHORT pic[ZMAX];
	SHORT slope[ZMAX];
	SHORT zcount;
} SAVE, *SAVEp;

extern SAVE save;

extern BOOL FAF_DebugView;
BOOL FAFon = 0;
BOOL FAF_DontMoveSectors = FALSE;

short bak_searchsector, bak_searchwall, bak_searchstat;
extern short searchsector, searchwall, searchstat, searchit;

VOID SetupBuildFAF(VOID);
VOID ResetBuildFAF(VOID);

void ToggleFAF(void)
{
	if (keystatus[KEYSC_3])
	{
		keystatus[KEYSC_3] = FALSE;

		FLIP(FAFon, 1);
		if (FAFon)
		{
			SetupBuildFAF();
		}
		else
		{
			ResetBuildFAF();
		}
	}

	if (FAFon && qsetmode == 200)
	{
		DrawOverlapRoom(posx, posy, posz, ang, horiz, cursectnum);

		// make it so that you can edit both areas in 3D
		// back up vars after the first drawrooms
		bak_searchsector = searchsector;
		bak_searchwall = searchwall;
		bak_searchstat = searchstat;
		searchit = 2;
	}

	if (FAFon && qsetmode == 200 && keystatus[KEYSC_4])
	{
		short match;
		long tx, ty, tz;
		short tsectnum;
		short i;
		keystatus[KEYSC_4] = FALSE;

		tx = posx;
		ty = posy;
		tz = posz;
		tsectnum = cursectnum;

		save.zcount = 0;

		if (sector[cursectnum].ceilingpicnum == FAF_MIRROR_PIC)
		{
			match = ViewSectorInScene(tsectnum, VIEW_THRU_CEILING, VIEW_LEVEL1);

			FAF_DontMoveSectors = TRUE;
			FindCeilingView(match, &tx, &ty, tz, &tsectnum);
			FAF_DontMoveSectors = FALSE;

			posx = tx;
			posy = ty;
			cursectnum = tsectnum;
			posz = sector[cursectnum].floorz - Z(20);
		}
		else
			if (sector[cursectnum].floorpicnum == FAF_MIRROR_PIC)
			{
				match = ViewSectorInScene(tsectnum, VIEW_THRU_FLOOR, VIEW_LEVEL2);

				FAF_DontMoveSectors = TRUE;
				FindFloorView(match, &tx, &ty, tz, &tsectnum);
				FAF_DontMoveSectors = FALSE;

				posx = tx;
				posy = ty;
				cursectnum = tsectnum;
				posz = sector[cursectnum].ceilingz + Z(20);
			}
	}


}


void FAF_AfterDrawRooms(void)
{
	// make it so that you can edit both areas in 3D
	// if your cursor is in the FAF_MIRROR_PIC area use the vars from the first
	// drawrooms instead
	if ((searchstat == 1 && sector[searchsector].ceilingpicnum == FAF_MIRROR_PIC) ||
		(searchstat == 2 && sector[searchsector].floorpicnum == FAF_MIRROR_PIC))
	{
		searchsector = bak_searchsector;
		searchwall = bak_searchwall;
		searchstat = bak_searchstat;
	}
}

VOID
SetupBuildFAF(VOID)
{
	short i, nexti;
	SPRITEp sp, vc_sp, vf_sp, vl_sp;
	short SpriteNum, NextSprite;
	short vc, nextvc, vf, nextvf, l, nextl;
	long zdiff;

	// move every sprite to the correct list
	TRAVERSE_SPRITE_STAT(headspritestat[STAT_DEFAULT], SpriteNum, NextSprite)
	{
		sp = &sprite[SpriteNum];

		if (sp->picnum != ST1)
			continue;

		switch (sp->hitag)
		{
		case VIEW_THRU_CEILING:
		case VIEW_THRU_FLOOR:
		{
			int i, nexti;
			// make sure there is only one set per level of these
			TRAVERSE_SPRITE_STAT(headspritestat[STAT_FAF], i, nexti)
			{
				if (sprite[i].hitag == sp->hitag && sprite[i].lotag == sp->lotag)
				{
					sprintf(ds, "Two VIEW_THRU_ tags with same match found on level\n1: x %ld, y %ld \n2: x %ld, y %ld", sp->x, sp->y, sprite[i].x, sprite[i].y);
					Message(ds, 0);
				}
			}

			changespritestat(SpriteNum, STAT_FAF);
			break;
		}

		case VIEW_LEVEL1:
		case VIEW_LEVEL2:
		case VIEW_LEVEL3:
		case VIEW_LEVEL4:
		case VIEW_LEVEL5:
		case VIEW_LEVEL6:
		{
			changespritestat(SpriteNum, STAT_FAF);
			break;
		}
		}
	}

	TRAVERSE_SPRITE_STAT(headspritestat[STAT_FAF], i, nexti)
	{
		sp = &sprite[i];

		if (sector[sp->sectnum].ceilingpicnum == FAF_PLACE_MIRROR_PIC)
		{
			sector[sp->sectnum].ceilingpicnum = FAF_MIRROR_PIC;
			if (sector[sp->sectnum].floorz == sector[sp->sectnum].ceilingz)
			{
				sprintf(ds, "Mirror used for non-connect area. Use tile 342. Sect %d, x %ld, y %ld\n", sp->sectnum, wall[sector[sp->sectnum].wallptr].x, wall[sector[sp->sectnum].wallptr].y);
				Message(ds, 0);
			}
		}

		if (sector[sp->sectnum].floorpicnum == FAF_PLACE_MIRROR_PIC)
		{
			sector[sp->sectnum].floorpicnum = FAF_MIRROR_PIC;
			if (sector[sp->sectnum].floorz == sector[sp->sectnum].ceilingz)
			{
				sprintf(ds, "Mirror used for non-connect area. Use tile 342. Sect %d, x %ld, y %ld\n", sp->sectnum, wall[sector[sp->sectnum].wallptr].x, wall[sector[sp->sectnum].wallptr].y);
				Message(ds, 0);
			}
		}

		if (sector[sp->sectnum].ceilingpicnum == FAF_PLACE_MIRROR_PIC + 1)
			sector[sp->sectnum].ceilingpicnum = FAF_MIRROR_PIC + 1;

		if (sector[sp->sectnum].floorpicnum == FAF_PLACE_MIRROR_PIC + 1)
			sector[sp->sectnum].floorpicnum = FAF_MIRROR_PIC + 1;
	}


	for (i = 0; i < numwalls; i++)
	{
		if (wall[i].picnum == FAF_PLACE_MIRROR_PIC)
			wall[i].picnum = FAF_MIRROR_PIC;

		if (wall[i].picnum == FAF_PLACE_MIRROR_PIC + 1)
			wall[i].picnum = FAF_MIRROR_PIC + 1;
	}

#if 0    
	// check ceiling and floor heights
	TRAVERSE_SPRITE_STAT(headspritestat[STAT_FAF], vc, nextvc)
	{
		vc_sp = &sprite[vc];

		if (vc_sp->hitag == VIEW_THRU_CEILING)
		{
			TRAVERSE_SPRITE_STAT(headspritestat[STAT_FAF], vf, nextvf)
			{
				vf_sp = &sprite[vf];

				if (vf_sp->hitag == VIEW_THRU_FLOOR && vf_sp->lotag == vc_sp->lotag)
				{
					zdiff = labs(sector[vc_sp->sectnum].ceilingz - sector[vf_sp->sectnum].floorz);

					//DSPRINTF(ds,"zdiff %d",zdiff);
					MONO_PRINT(ds);

					TRAVERSE_SPRITE_STAT(headspritestat[STAT_FAF], l, nextl)
					{
						vl_sp = &sprite[l];

						if (vl_sp->hitag == VIEW_LEVEL1)
						{
							if (sector[vl_sp->sectnum].ceilingz < sector[vc_sp->sectnum].ceilingz + zdiff)
							{
								sprintf(ds, "Sector %d (x %d, y %d) ceiling z to close to VIEW_THRU_CEILING z",
									vl_sp->sectnum, wall[sector[vl_sp->sectnum].wallptr].x, wall[sector[vl_sp->sectnum].wallptr].y);
								Message(ds, 0);
							}
						}
						else
							if (vl_sp->hitag == VIEW_LEVEL2)
							{
								if (sector[vl_sp->sectnum].floorz > sector[vf_sp->sectnum].floorz + zdiff)
								{
									sprintf(ds, "Sector %d (x %d, y %d)floor z to close to VIEW_THRU_FLOOR z",
										vl_sp->sectnum, wall[sector[vl_sp->sectnum].wallptr].x, wall[sector[vl_sp->sectnum].wallptr].y);
									Message(ds, 0);
								}
							}
					}
				}
			}
		}
	}
#endif

}

VOID
ResetBuildFAF(VOID)
{
	short i, nexti;
	SPRITEp sp;

	TRAVERSE_SPRITE_STAT(headspritestat[STAT_FAF], i, nexti)
	{
		sp = &sprite[i];

		if (sector[sp->sectnum].ceilingpicnum == FAF_MIRROR_PIC)
			sector[sp->sectnum].ceilingpicnum = FAF_PLACE_MIRROR_PIC;

		if (sector[sp->sectnum].floorpicnum == FAF_MIRROR_PIC)
			sector[sp->sectnum].floorpicnum = FAF_PLACE_MIRROR_PIC;

		if (sector[sp->sectnum].ceilingpicnum == FAF_MIRROR_PIC + 1)
			sector[sp->sectnum].ceilingpicnum = FAF_PLACE_MIRROR_PIC + 1;

		if (sector[sp->sectnum].floorpicnum == FAF_MIRROR_PIC + 1)
			sector[sp->sectnum].floorpicnum = FAF_PLACE_MIRROR_PIC + 1;
	}

	for (i = 0; i < numwalls; i++)
	{
		if (wall[i].picnum == FAF_MIRROR_PIC)
			wall[i].picnum = FAF_PLACE_MIRROR_PIC;

		if (wall[i].picnum == FAF_MIRROR_PIC + 1)
			wall[i].picnum = FAF_PLACE_MIRROR_PIC + 1;
	}

	TRAVERSE_SPRITE_STAT(headspritestat[STAT_FAF], i, nexti)
	{
		changespritestat(i, STAT_DEFAULT);
	}
}



BOOL
SectorInScene(short tile_num)
{
	if (TEST(gotsector[tile_num >> 3], 1 << (tile_num & 7)))
	{
		RESET(gotsector[tile_num >> 3], 1 << (tile_num & 7));
		return (TRUE);
	}

	return (FALSE);
}
