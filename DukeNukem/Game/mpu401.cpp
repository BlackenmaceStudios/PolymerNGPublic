//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

/**********************************************************************
   module: MPU401.C

   author: James R. Dose
   date:   January 1, 1994

   Low level routines to support sending of MIDI data to MPU401
   compatible MIDI interfaces.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/
#include "pch.h"
#include "mpu401.h"
#include "compat.h"
#include "pragmas.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>


typedef struct
{
    int32_t time;
    int32_t stream;
    int32_t event;
}
MIDIEVENTHEAD;
#define PAD(x) ((((x)+3)&(~3)))

#define BUFFERLEN (32*4*4)
#define NUMBUFFERS 6
static char eventbuf[NUMBUFFERS][BUFFERLEN];
static int32_t  eventcnt[NUMBUFFERS];

int32_t  _MPU_CurrentBuffer = 0;
int32_t  _MPU_BuffersWaiting = 0;

extern uint32_t _MIDI_GlobalPositionInTicks;
uint32_t _MPU_LastEvent=0;

#define MIDI_NOTE_OFF         0x80
#define MIDI_NOTE_ON          0x90
#define MIDI_POLY_AFTER_TCH   0xA0
#define MIDI_CONTROL_CHANGE   0xB0
#define MIDI_PROGRAM_CHANGE   0xC0
#define MIDI_AFTER_TOUCH      0xD0
#define MIDI_PITCH_BEND       0xE0
#define MIDI_META_EVENT       0xFF
#define MIDI_END_OF_TRACK     0x2F
#define MIDI_TEMPO_CHANGE     0x51
#define MIDI_MONO_MODE_ON     0x7E
#define MIDI_ALL_NOTES_OFF    0x7B


/**********************************************************************

   Memory locked functions:

**********************************************************************/


void MPU_FinishBuffer(int32_t buffer)
{
   
}

void MPU_BeginPlayback(void)
{

}

void MPU_Pause(void)
{
   
}

void MPU_Unpause(void)
{
   
}



/*---------------------------------------------------------------------
   Function: MPU_SendMidi

   Queues a MIDI message to the music device.
---------------------------------------------------------------------*/

int32_t MPU_GetNextBuffer(void)
{

    return -1;
}

void MPU_SendMidi(char *data, int32_t count)
{
}


/*---------------------------------------------------------------------
   Function: MPU_SendMidiImmediate

   Sends a MIDI message immediately to the the music device.
---------------------------------------------------------------------*/
void MPU_SendMidiImmediate(char *data, int32_t count)
{
   
}


/*---------------------------------------------------------------------
   Function: MPU_Reset

   Resets the MPU401 card.
---------------------------------------------------------------------*/

int32_t MPU_Reset
(
    void
)
{

    return(MPU_Ok);
}


/*---------------------------------------------------------------------
   Function: MPU_Init

   Detects and initializes the MPU401 card.
---------------------------------------------------------------------*/

int32_t MPU_Init
(
    int32_t addr
)

{

    return(MPU_Ok);
}


/*---------------------------------------------------------------------
   Function: MPU_NoteOff

   Sends a full MIDI note off event out to the music device.
---------------------------------------------------------------------*/

void MPU_NoteOff
(
    int32_t channel,
    int32_t key,
    int32_t velocity
)

{

}


/*---------------------------------------------------------------------
   Function: MPU_NoteOn

   Sends a full MIDI note on event out to the music device.
---------------------------------------------------------------------*/

void MPU_NoteOn
(
    int32_t channel,
    int32_t key,
    int32_t velocity
)

{

}


/*---------------------------------------------------------------------
   Function: MPU_PolyAftertouch

   Sends a full MIDI polyphonic aftertouch event out to the music device.
---------------------------------------------------------------------*/

void MPU_PolyAftertouch
(
    int32_t channel,
    int32_t key,
    int32_t pressure
)

{

}


/*---------------------------------------------------------------------
   Function: MPU_ControlChange

   Sends a full MIDI control change event out to the music device.
---------------------------------------------------------------------*/

void MPU_ControlChange
(
    int32_t channel,
    int32_t number,
    int32_t value
)

{

}


/*---------------------------------------------------------------------
   Function: MPU_ProgramChange

   Sends a full MIDI program change event out to the music device.
---------------------------------------------------------------------*/

void MPU_ProgramChange
(
    int32_t channel,
    int32_t program
)

{

}


/*---------------------------------------------------------------------
   Function: MPU_ChannelAftertouch

   Sends a full MIDI channel aftertouch event out to the music device.
---------------------------------------------------------------------*/

void MPU_ChannelAftertouch
(
    int32_t channel,
    int32_t pressure
)

{

}


/*---------------------------------------------------------------------
   Function: MPU_PitchBend

   Sends a full MIDI pitch bend event out to the music device.
---------------------------------------------------------------------*/

void MPU_PitchBend
(
    int32_t channel,
    int32_t lsb,
    int32_t msb
)

{

}



void MPU_SetTempo(int32_t tempo)
{

}

void MPU_SetDivision(int32_t division)
{

}

void MPU_SetVolume(int32_t volume)
{
   
}

int32_t MPU_GetVolume(void)
{

    return 0;
}

