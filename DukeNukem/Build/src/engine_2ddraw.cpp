#if (PNG_LIBPNG_VER > 10599)
# include <string.h>
#endif
#include "compat.h"
#include "build3d.h"
#include "PolymerNG/Renderer/Renderer.h"
#include "build.h"
//#include "editor.h"
#include "pragmas.h"
#include "cache1d.h"
#include "a.h"
#include "osd.h"
#include "crc32.h"
#include "xxhash.h"
#include "lz4.h"
#include "colmatch.h"

#include "baselayer.h"
#include "scriptfile.h"

static int _2dWindowWidth[2];
static int _2dWindowHeight[2];

byte *_2dbuffer[2];
static bool smpFrame = false;

void begindrawing2D(void)
{
	if (bpp > 8)
	{
		if (offscreenrendering) return;
		frameplace = 0;
		bytesperline = 0;
		return;
	}


	if (offscreenrendering) return;

	if (xres != _2dWindowWidth[smpFrame] || yres != _2dWindowHeight[smpFrame])
	{
		_2dbuffer[smpFrame] = (byte *)realloc(_2dbuffer[smpFrame], xres * yres);
		_2dWindowWidth[smpFrame] = xres;
		_2dWindowHeight[smpFrame] = yres;
	}


	frameplace = (intptr_t)_2dbuffer[smpFrame];
	memset(_2dbuffer[smpFrame], 0, _2dWindowWidth[smpFrame] * _2dWindowHeight[smpFrame]);

	bytesperline = xres | 1;

	calc_ylookup(bytesperline, ydim);
}


//
// enddrawing() -- unlocks the framebuffer
//
void enddrawing2D(void)
{
	if (bpp > 8)
	{
		if (!offscreenrendering) frameplace = 0;
		return;
	}

	if (!frameplace) return;
	if (!offscreenrendering) frameplace = 0;

	BuildRenderCommand command;
	command.taskId = BUILDRENDER_TASK_DRAWCLASSICSCREEN;
	command.taskDrawClassicScreen.width = _2dWindowWidth[smpFrame];
	command.taskDrawClassicScreen.height = _2dWindowHeight[smpFrame];
	command.taskDrawClassicScreen.screen_buffer = _2dbuffer[smpFrame];
	renderer.AddRenderCommand(command);

	smpFrame = !smpFrame;

	nextpage();
}

void begindrawing()
{
	frameplace = (intptr_t)_2dbuffer[smpFrame];

	bytesperline = xres;

	calc_ylookup(bytesperline, ydim);
}

void enddrawing()
{

}