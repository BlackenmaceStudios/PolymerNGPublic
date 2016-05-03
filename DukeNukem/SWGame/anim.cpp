//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

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

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#include "pch.h"
#include "build.h"
#include "compat.h"
#include "cache1d.h"

#include "keys.h"
#include "mytypes.h"
#include "develop.h"
#include "fx_man.h"
#include "music.h"
#include "scriplib.h"
#include "file_lib.h"
#include "gamedefs.h"
#include "keyboard.h"
#include "util_lib.h"

#include "control.h"
#include "config.h"
#include "sounds.h"
#include "function.h"

#include "game.h"
#include "colormap.h"
#include "net.h"

#include "animlib.h"
#include "anim.h"

#define MAX_ANMS 10
#define TILE_ANIM           (MAXTILES-4)
# pragma pack(push,1)

/* structure declarations for deluxe animate large page files, doesn't
need to be in the header because there are no exposed functions
that use any of this directly */

typedef struct lpfileheaderstruct
{
    uint32_t id;              /* 4 uint8_tacter ID == "LPF " */
    uint16_t maxLps;          /* max # largePages allowed. 256 FOR NOW.   */
    uint16_t nLps;            /* # largePages in this file. */
    uint32_t nRecords;        /* # records in this file.  65534 is current limit + ring */
    uint16_t maxRecsPerLp;    /* # records permitted in an lp. 256 FOR NOW.   */
    uint16_t lpfTableOffset;  /* Absolute Seek position of lpfTable.  1280 FOR NOW. */
    uint32_t contentType;     /* 4 character ID == "ANIM" */
    uint16_t width;           /* Width of screen in pixels. */
    uint16_t height;          /* Height of screen in pixels. */
    uint8_t variant;          /* 0==ANIM. */
    uint8_t version;          /* 0==frame rate in 18/sec, 1= 70/sec */
    uint8_t hasLastDelta;     /* 1==Last record is a delta from last-to-first frame. */
    uint8_t lastDeltaValid;   /* 0==Ignore ring frame. */
    uint8_t pixelType;        /* 0==256 color. */
    uint8_t CompressionType;  /* 1==(RunSkipDump) Only one used FOR NOW. */
    uint8_t otherRecsPerFrm;  /* 0 FOR NOW. */
    uint8_t bitmaptype;       /* 1==320x200, 256-color.  Only one implemented so far. */
    uint8_t recordTypes[32];  /* Not yet implemented. */
    uint32_t nFrames;         /* Number of actual frames in the file, includes ring frame. */
    uint16_t framesPerSecond; /* Number of frames to play per second. */
    uint16_t pad2[29];        /* 58 bytes of filler to round up to 128 bytes total. */
}
lpfileheader;                 /* (comments from original source) */

// this is the format of a large page structure
typedef struct
{
    uint16_t baseRecord;   // Number of first record in this large page.
    uint16_t nRecords;        // Number of records in lp.
    // bit 15 of "nRecords" == "has continuation from previous lp".
    // bit 14 of "nRecords" == "final record continues on next lp".
    uint16_t nBytes;                  // Total number of bytes of contents, excluding header.
} lp_descriptor;

#pragma pack(pop)

#define IMAGEBUFFERSIZE 0x10000

typedef struct
{
    uint16_t framecount;          // current frame of anim
    lpfileheader * lpheader;           // file header will be loaded into this structure
    lp_descriptor * LpArray; // arrays of large page structs used to find frames
    uint16_t curlpnum;               // initialize to an invalid Large page number
    lp_descriptor * curlp;        // header of large page currently in memory
    uint16_t * thepage;     // buffer where current large page is loaded
    uint8_t imagebuffer[IMAGEBUFFERSIZE]; // buffer where anim frame is decoded
    uint8_t * buffer;
    uint8_t pal[768];
    int32_t  currentframe;
} anim_t;

anim_t *anm_ptr[MAX_ANMS];

long length;
long ANIMnumframes;
char ANIMpal[3 * 256];
char ANIMnum = 0;
short SoundState;

char *ANIMname[] =
{
    "sw.anm",
    "swend.anm",
    "sumocinm.anm",
    "zfcin.anm",
};

#define ANIM_TILE(num) (MAXTILES-11 + (num))

VOID AnimShareIntro ( int frame, int numframes )
{
    long zero = 0;
    
    if ( frame == numframes - 1 )
    {
        ototalclock += 120;
    }
    
    else if ( frame == 1 )
    {
        PlaySound ( DIGI_NOMESSWITHWANG, &zero, &zero, &zero, v3df_none );
        ototalclock += 120 * 3;
    }
    
    else
    {
        ototalclock += 8;
    }
    
    if ( frame == 5 )
    {
        PlaySound ( DIGI_INTRO_SLASH, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 15 )
    {
        PlaySound ( DIGI_INTRO_WHIRL, &zero, &zero, &zero, v3df_none );
    }
}

VOID AnimSerp ( int frame, int numframes )
{
    long zero = 0;
    ototalclock += 16;
    
    if ( frame == numframes - 1 )
    {
        ototalclock += 1 * 120;
    }
    
    if ( frame == 1 )
    {
        PlaySound ( DIGI_SERPTAUNTWANG, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 16 )
    {
        PlaySound ( DIGI_SHAREND_TELEPORT, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 35 )
    {
        SoundState++;
        PlaySound ( DIGI_WANGTAUNTSERP1, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 51 )
    {
        SoundState++;
        PlaySound ( DIGI_SHAREND_UGLY1, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 64 )
    {
        SoundState++;
        PlaySound ( DIGI_SHAREND_UGLY2, &zero, &zero, &zero, v3df_none );
    }
}

VOID AnimSumo ( int frame, int numframes )
{
    long zero = 0;
    ototalclock += 10;
    
    if ( frame == numframes - 1 )
    {
        ototalclock += 1 * 120;
    }
    
    if ( frame == 1 )
    {
        ototalclock += 30;
    }
    
    if ( frame == 2 )
    {
        // hungry
        PlaySound ( DIGI_JG41012, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 30 )
    {
        PlaySound ( DIGI_HOTHEADSWITCH, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 42 )
    {
        PlaySound ( DIGI_HOTHEADSWITCH, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 59 )
    {
        PlaySound ( DIGI_JG41028, &zero, &zero, &zero, v3df_none );
    }
}

VOID AnimZilla ( int frame, int numframes )
{
    long zero = 0;
    ototalclock += 16;
    
    if ( frame == numframes - 1 )
    {
        ototalclock += 1 * 120;
    }
    
    if ( frame == 1 )
    {
        PlaySound ( DIGI_ZC1, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 5 )
    {
        PlaySound ( DIGI_JG94024, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 14 )
    {
        PlaySound ( DIGI_ZC2, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 30 )
    {
        PlaySound ( DIGI_ZC3, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 32 )
    {
        PlaySound ( DIGI_ZC4, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 37 )
    {
        PlaySound ( DIGI_ZC5, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 63 )
    {
        PlaySound ( DIGI_Z16043, &zero, &zero, &zero, v3df_none );
        PlaySound ( DIGI_ZC6, &zero, &zero, &zero, v3df_none );
        PlaySound ( DIGI_ZC7, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 72 )
    {
        PlaySound ( DIGI_ZC7, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 73 )
    {
        PlaySound ( DIGI_ZC4, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 77 )
    {
        PlaySound ( DIGI_ZC5, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 87 )
    {
        PlaySound ( DIGI_ZC8, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 103 )
    {
        PlaySound ( DIGI_ZC7, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 108 )
    {
        PlaySound ( DIGI_ZC9, &zero, &zero, &zero, v3df_none );
    }
    
    else if ( frame == 120 )
    {
        PlaySound ( DIGI_JG94039, &zero, &zero, &zero, v3df_none );
    }
}

char * LoadAnm ( short anim_num )
{
    long handle;
    char *animbuf, *palptr;
    long i, j, k;
    DSPRINTF ( ds, "LoadAnm" );
    MONO_PRINT ( ds );
    // this seperate allows the anim to be precached easily
    ANIMnum = anim_num;
    // lock it
    walock[ANIM_TILE ( ANIMnum )] = 219;
    
    if ( anm_ptr[anim_num] == 0 )
    {
        handle = kopen4load ( ANIMname[ANIMnum], 0 );
        
        if ( handle == -1 )
        {
            return ( NULL );
        }
        
        length = kfilelength ( handle );
        allocache ( ( intptr_t * ) &anm_ptr[anim_num], length + sizeof ( anim_t ), &walock[ANIM_TILE ( ANIMnum )] );
        animbuf = ( char * ) ( FP_OFF ( anm_ptr[anim_num] ) + sizeof ( anim_t ) );
        kread ( handle, animbuf, length );
        kclose ( handle );
    }
    
    else
    {
        animbuf = ( char * ) ( FP_OFF ( anm_ptr[anim_num] ) + sizeof ( anim_t ) );
    }
    
    return ( animbuf );
}

void
playanm ( short anim_num )
{
    char *animbuf, *palptr;
    long i, j, k, numframes = 0;
    int32 handle = -1;
    char ANIMvesapal[4 * 256];
    char tempbuf[256];
    char *palook_bak = palookup[0];
    ANIMnum = anim_num;
    KB_FlushKeyboardQueue();
    KB_ClearKeysDown();
    DSPRINTF ( ds, "PlayAnm" );
    MONO_PRINT ( ds );
    DSPRINTF ( ds, "PlayAnm" );
    MONO_PRINT ( ds );
    animbuf = LoadAnm ( anim_num );
    
    if ( !animbuf )
    {
        return;
    }
    
    DSPRINTF ( ds, "PlayAnm - Palette Stuff" );
    MONO_PRINT ( ds );
    
    for ( i = 0; i < 256; i++ )
    {
        tempbuf[i] = i;
    }
    
    palookup[0] = tempbuf;
    ANIM_LoadAnim ( ( uint8_t * ) animbuf, length );
    ANIMnumframes = ANIM_NumFrames();
    numframes = ANIMnumframes;
    palptr = ( char * ) ANIM_GetPalette();
    
    for ( i = 0; i < 768; i++ )
    {
        ANIMvesapal[i] = palptr[i] >> 2;
    }
    
    tilesiz[TILE_ANIM].x = 200;
    tilesiz[TILE_ANIM].y = 320;
    setbasepal ( 0, ( const uint8_t * ) ANIMvesapal );
    setbrightness ( gs.Brightness, 0, 2 );
    
    if ( ANIMnum == 1 )
    {
        // draw the first frame
        waloff[TILE_ANIM] = FP_OFF ( ANIM_DrawFrame ( 1 ) );
        invalidatetile ( TILE_ANIM, 0, 1 << 4 );
        rotatesprite ( 0 << 16, 0 << 16, 65536L, 512, TILE_ANIM, 0, 0, 2 + 4 + 8 + 16 + 64, 0, 0, xdim - 1, ydim - 1 );
    }
    
    SoundState = 0;
    //ototalclock = totalclock + 120*2;
    ototalclock = totalclock;
    
    for ( i = 1; i < numframes; i++ )
    {
        while ( totalclock < ototalclock )
        {
            switch ( ANIMnum )
            {
                case 0:
                    if ( KB_KeyWaiting() )
                    {
                        goto ENDOFANIMLOOP;
                    }
                    
                    break;
                    
                case 1:
                    if ( KEY_PRESSED ( KEYSC_ESC ) )
                    {
                        goto ENDOFANIMLOOP;
                    }
                    
                    break;
            }
            
            getpackets();
        }
        
        switch ( ANIMnum )
        {
            case ANIM_INTRO:
                AnimShareIntro ( i, numframes );
                break;
                
            case ANIM_SERP:
                AnimSerp ( i, numframes );
                break;
                
            case ANIM_SUMO:
                AnimSumo ( i, numframes );
                break;
                
            case ANIM_ZILLA:
                AnimZilla ( i, numframes );
                break;
        }
        
        waloff[TILE_ANIM] = FP_OFF ( ANIM_DrawFrame ( i ) );
        invalidatetile ( TILE_ANIM, 0, 1 << 4 );
        rotatesprite ( 0 << 16, 0 << 16, 65536L, 512, TILE_ANIM, 0, 0, 2 + 4 + 8 + 16 + 64, 0, 0, xdim - 1, ydim - 1 );
        nextpage();
    }
    
    // pause on final frame
    while ( totalclock < ototalclock )
    {
        getpackets();
    }
    
ENDOFANIMLOOP:
    clearview ( 0 );
    nextpage();
    palookup[0] = palook_bak;
    //    setbrightness(gs.Brightness, (char*)palette_data, 2);
    setbasepal ( 0, ( const uint8_t * ) palette_data );
    setbrightness ( gs.Brightness, 0, 2 );
    KB_FlushKeyboardQueue();
    KB_ClearKeysDown();
    ANIM_FreeAnim();
    walock[ANIM_TILE ( ANIMnum )] = 1;
}
