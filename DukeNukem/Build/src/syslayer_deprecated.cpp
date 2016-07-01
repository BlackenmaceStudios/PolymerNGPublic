// syslayer_deprecated.cpp
//

// shit i want to remove

#include <malloc.h>

#ifdef _MSC_VER
#include <InitGuid.h>
#endif

#include <windows.h>

#ifndef DIK_PAUSE
# define DIK_PAUSE 0xC5
#endif

#include <math.h>  // pow

#ifdef __cplusplus
#include <algorithm>
#endif

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#define __STDC_FORMAT_MACROS
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>


#include "compat.h"
#include "winlayer.h"
#include "baselayer.h"
#include "pragmas.h"
#include "build.h"
#include "a.h"
#include "osd.h"
#include "rawinput.h"
#include "mutex.h"

#include "winbits.h"
#include "engine_priv.h"

// undefine to restrict windowed resolutions to conventional sizes
#define ANY_WINDOWED_SIZE

static mutex_t m_initprintf;
static int32_t winlayer_have_ATI = 0;

static int32_t   _buildargc = 0;
static const char **_buildargv = NULL;
static char *argvbuf = NULL;

// Windows crud
static HINSTANCE hInstance = 0;
static HWND hWindow = 0;
#define WINDOW_STYLE (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX)
static BOOL window_class_registered = FALSE;

//static DDGAMMARAMP sysgamma;
extern int32_t curbrightness, gammabrightness;

#ifdef USE_OPENGL
// OpenGL stuff
static HGLRC hGLRC = 0;
int32_t nofog = 0;
char nogl = 0;
char forcegl = 0;
#endif

static const char *GetDDrawError(HRESULT code);
static const char *GetDInputError(HRESULT code);
static void ShowDDrawErrorBox(const char *m, HRESULT r);
static void ShowDInputErrorBox(const char *m, HRESULT r);
static LRESULT CALLBACK WndProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL InitDirectDraw(void);
static void UninitDirectDraw(void);
static int32_t RestoreDirectDrawMode(void);
static void ReleaseDirectDrawSurfaces(void);
static BOOL InitDirectInput(void);
static void UninitDirectInput(void);
static void GetKeyNames(void);
static void AcquireInputDevices(char acquire);
static inline void DI_PollJoysticks(void);
static int32_t SetupDirectDraw(int32_t width, int32_t height);
static void UninitDIB(void);
static int32_t SetupDIB(int32_t width, int32_t height);
#ifdef USE_OPENGL
static void ReleaseOpenGL(void);
static void UninitOpenGL(void);
static int32_t SetupOpenGL(int32_t width, int32_t height, int32_t bitspp);
#endif
static BOOL RegisterWindowClass(void);
static BOOL CreateAppWindow(int32_t modenum);
static void DestroyAppWindow(void);

// video
static int32_t desktopxdim = 0;
static int32_t desktopydim = 0;
static int32_t desktopbpp = 0;
static int32_t modesetusing = -1;
static int32_t curvidmode = -1;
static int32_t customxdim = 640;
static int32_t customydim = 480;
static int32_t custombpp = 8;
static int32_t customfs = 0;
static uint32_t modeschecked = 0;
int32_t xres = -1;
int32_t yres = -1;
int32_t fullscreen = 0;
int32_t bpp = 0;
int32_t bytesperline = 0;
int32_t lockcount = 0;
int32_t glcolourdepth = 32;
#ifdef USE_OPENGL
static int32_t vsync_render = 0;
#endif
uint32_t maxrefreshfreq = 60;
intptr_t frameplace = 0;
char modechange = 1;
char repaintneeded = 0;
char offscreenrendering = 0;
char videomodereset = 0;

// input and events
char quitevent = 0;
char appactive = 1;
char realfs = 0;
char regrabmouse = 0;
char defaultlayoutname[KL_NAMELENGTH];
int32_t inputchecked = 0;

//-------------------------------------------------------------------------------------------------
//  DINPUT (JOYSTICK)
//=================================================================================================

#define JOYSTICK	0

//static HMODULE               hDInputDLL = NULL;
//static LPDIRECTINPUT7A        lpDI = NULL;
//static LPDIRECTINPUTDEVICE7A lpDID = NULL;
#define INPUT_BUFFER_SIZE	32
static GUID                  guidDevs;

char di_disabled = 0;
static char di_devacquired;
static HANDLE di_inputevt = 0;
static int32_t joyblast = 0;

//static struct
//{
//	char *name;
//	LPDIRECTINPUTDEVICE7A *did;
//	LPCDIDATAFORMAT df;
//} devicedef = { "joystick", &lpDID, &c_dfDIJoystick };

static struct _joydef
{
	char *name;
	uint32_t ofs;	// directinput 'dwOfs' value
} *axisdefs = NULL, *buttondefs = NULL, *hatdefs = NULL;


//
// handleevents() -- process the Windows message queue
//   returns !0 if there was an important event worth checking (like quitting)
//
int32_t handleevents(void)
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
//  TIMER
//=================================================================================================

static int32_t timerlastsample = 0;
int32_t timerticspersec = 0;
static double msperu64tick = 0;
static void(*usertimercallback)(void) = NULL;

//  This timer stuff is all Ken's idea.

//
// installusertimercallback() -- set up a callback function to be called when the timer is fired
//
void(*installusertimercallback(void(*callback)(void)))(void)
{
	void(*oldtimercallback)(void);

	oldtimercallback = usertimercallback;
	usertimercallback = callback;

	return oldtimercallback;
}


//
// inittimer() -- initialize timer
//
int32_t inittimer(int32_t tickspersecond)
{
	int64_t t;

	if (win_timerfreq) return 0;	// already installed

									//    initprintf("Initializing timer\n");

	t = win_inittimer();
	if (t < 0)
		return t;

	timerticspersec = tickspersecond;
	QueryPerformanceCounter((LARGE_INTEGER *)&t);
	timerlastsample = (int32_t)(t*timerticspersec / win_timerfreq);

	usertimercallback = NULL;

	msperu64tick = 1000.0 / (double)getu64tickspersec();

	return 0;
}

//
// uninittimer() -- shut down timer
//
void uninittimer(void)
{
	if (!win_timerfreq) return;

	win_timerfreq = 0;
	timerticspersec = 0;

	msperu64tick = 0;
}

//
// sampletimer() -- update totalclock
//
void sampletimer(void)
{
	int64_t i;
	int32_t n;

	if (!win_timerfreq) return;

	QueryPerformanceCounter((LARGE_INTEGER *)&i);
	n = (int32_t)((i*timerticspersec / win_timerfreq) - timerlastsample);

	if (n <= 0) return;

	totalclock += n;
	timerlastsample += n;

	if (usertimercallback) for (; n > 0; n--) usertimercallback();
}


//
// getticks() -- returns the windows ticks count
//
uint32_t getticks(void)
{
	int64_t i;
	if (win_timerfreq == 0) return 0;
	QueryPerformanceCounter((LARGE_INTEGER *)&i);
	return (uint32_t)(i*longlong(1000) / win_timerfreq);
}

// high-resolution timers for profiling
uint64_t getu64ticks(void)
{
	return win_getu64ticks();
}

uint64_t getu64tickspersec(void)
{
	return win_timerfreq;
}

// Returns the time since an unspecified starting time in milliseconds.
ATTRIBUTE((flatten))
double gethiticks(void)
{
	return (double)getu64ticks() * msperu64tick;
}

//
// gettimerfreq() -- returns the number of ticks per second the timer is configured to generate
//
int32_t gettimerfreq(void)
{
	return timerticspersec;
}

//
// wm_msgbox/wm_ynbox() -- window-manager-provided message boxes
//
int32_t wm_msgbox(const char *name, const char *fmt, ...)
{
	return 0;
}


int32_t wm_ynbox(const char *name, const char *fmt, ...)
{
	return 0;
}

//
// system_getcvars() -- propagate any cvars that are read post-initialization
//
void system_getcvars(void)
{

}

//
// setjoydeadzone() -- sets the dead and saturation zones for the joystick
//
void setjoydeadzone(int32_t axis, uint16_t dead, uint16_t satur)
{

}

int32_t handleevents_peekkeys(void)
{
	return 0;
}

void idle_waitevent_timeout(uint32_t timeout)
{

}


int32_t initsystem(void)
{
	return 0;
}

//
// uninitsystem() -- uninit systems
//
void uninitsystem(void)
{

}

int32_t setvideomode(int32_t x, int32_t y, int32_t c, int32_t fs)
{
	return 0;
}

void getvalidmodes(void)
{

}

//
// showframe() -- update the display
//
void showframe(int32_t w)
{

}

int32_t setpalette(int32_t start, int32_t num)
{
	return 0;
}

int32_t setgamma(void)
{
	return 0;
}

void releaseallbuttons(void)
{

}

//
// initinput() -- init input system
//
int32_t initinput(void)
{
	return 0;
}

//
// uninitinput() -- uninit input system
//
void uninitinput(void)
{

}

const char *getjoyname(int32_t what, int32_t num)
{
	return NULL;
}

//
// resetvideomode() -- resets the video system
//
void resetvideomode(void)
{

}

//
// initputs() -- prints a string to the intitialization window
//
void initputs(const char *buf)
{

}

int32_t checkvideomode(int32_t *x, int32_t *y, int32_t c, int32_t fs, int32_t forced)
{
	return 1;
}