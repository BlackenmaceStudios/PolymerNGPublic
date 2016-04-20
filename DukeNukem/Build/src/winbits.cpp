// Windows layer-independent code

#include "compat.h"
#include "build.h"
#include "baselayer.h"
#include "osd.h"
#include "cache1d.h"



#include "winbits.h"

#ifdef BITNESS64
# define EBACKTRACEDLL "ebacktrace1-64.dll"
#else
# define EBACKTRACEDLL "ebacktrace1.dll"
#endif

int32_t backgroundidle = 1;

int64_t win_timerfreq = 0;

char silentvideomodeswitch = 0;

static char taskswitching = 1;

static HANDLE instanceflag = NULL;

static OSVERSIONINFOEX osv;

static int32_t togglecomp = 1;

FARPROC pwinever;

//
// CheckWinVersion() -- check to see what version of Windows we happen to be running under
//
BOOL CheckWinVersion(void)
{

    return FALSE;
}

static void win_printversion(void)
{
  
}

//
// win_allowtaskswitching() -- captures/releases alt+tab hotkeys
//
void win_allowtaskswitching(int32_t onf)
{
    if (onf == taskswitching) return;
    taskswitching = onf;

//    if (onf)
//    {
//        UnregisterHotKey(0,0);
//        UnregisterHotKey(0,1);
//    }
//    else
//    {
//        RegisterHotKey(0,0,MOD_ALT,VK_TAB);
//        RegisterHotKey(0,1,MOD_ALT|MOD_SHIFT,VK_TAB);
//    }

}


//
// win_checkinstance() -- looks for another instance of a Build app
//
int32_t win_checkinstance(void)
{
	return 0;
}

//
// high-resolution timers for profiling
//
int32_t win_inittimer(void)
{
    int64_t t;

    if (win_timerfreq) return 0;	// already installed

    // OpenWatcom seems to want us to query the value into a local variable
    // instead of the global 'win_timerfreq' or else it gets pissed with an
    // access violation
    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&t))
    {
        ShowErrorBox("Failed fetching timer frequency");
        return -1;
    }
    win_timerfreq = t;

    return 0;
}

uint64_t win_getu64ticks(void)
{
    uint64_t i;
    if (win_timerfreq == 0) return 0;
    QueryPerformanceCounter((LARGE_INTEGER *)&i);
    return i;
}



static void ToggleDesktopComposition(BOOL compEnable)
{
//    static HMODULE              hDWMApiDLL        = NULL;
//    static HRESULT(WINAPI *aDwmEnableComposition)(UINT);
//
//    if (!hDWMApiDLL && (hDWMApiDLL = LoadLibrary("DWMAPI.DLL")))
//        aDwmEnableComposition = (HRESULT(WINAPI *)(UINT))GetProcAddress(hDWMApiDLL, "DwmEnableComposition");
//
//    if (aDwmEnableComposition)
//    {
//        aDwmEnableComposition(compEnable);
//        if (!silentvideomodeswitch)
//            initprintf("%sabling desktop composition...\n", (compEnable) ? "En" : "Dis");
//    }
}

typedef void (*dllSetString)(const char*);

//
// win_open(), win_init(), win_setvideomode(), win_uninit(), win_close() -- shared code
//
void win_open(void)
{
#ifdef DEBUGGINGAIDS
    HMODULE ebacktrace = LoadLibraryA(EBACKTRACEDLL);
    if (ebacktrace)
    {
        dllSetString SetTechnicalName = (dllSetString) GetProcAddress(ebacktrace, "SetTechnicalName");
        dllSetString SetProperName = (dllSetString) GetProcAddress(ebacktrace, "SetProperName");

        if (SetTechnicalName)
            SetTechnicalName(AppTechnicalName);

        if (SetProperName)
            SetProperName(AppProperName);
    }
#endif

//    instanceflag = CreateSemaphore(NULL, 1,1, WindowClass);
}

void win_init(void)
{
    uint32_t i;

    cvar_t cvars_win[] =
    {
        { "r_togglecomposition","enable/disable toggle of desktop composition when initializing screen modes",(void *) &togglecomp, CVAR_BOOL, 0, 1 },
    };

    for (i=0; i<ARRAY_SIZE(cvars_win); i++)
    {
        if (OSD_RegisterCvar(&cvars_win[i]))
            continue;

        OSD_RegisterFunction(cvars_win[i].name, cvars_win[i].desc, osdcmd_cvar_set);
    }

    win_printversion();
}

void win_setvideomode(int32_t c)
{
    if (togglecomp && osv.dwMajorVersion == 6 && osv.dwMinorVersion < 2)
        ToggleDesktopComposition(c < 16);
}

void win_uninit(void)
{
    win_allowtaskswitching(1);
}

void win_close(void)
{
   // if (instanceflag) CloseHandle(instanceflag);
}


//
// ShowErrorBox() -- shows an error message box
//
void ShowErrorBox(const char *m)
{
//    TCHAR msg[1024];
//
//    wsprintf(msg, "%s: %s", m, GetWindowsErrorMsg(GetLastError()));
//    MessageBox(0, msg, apptitle, MB_OK|MB_ICONSTOP);
}

//
// GetWindowsErrorMsg() -- gives a pointer to a static buffer containing the Windows error message
//
LPTSTR GetWindowsErrorMsg(DWORD code)
{
    static TCHAR lpMsgBuf[1024];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, code,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)lpMsgBuf, 1024, NULL);

    return lpMsgBuf;
}


//
// Miscellaneous
//

int32_t addsearchpath_ProgramFiles(const char *p)
{
//    int32_t returncode = -1, i;
//    const char *ProgramFiles[2] = { Bgetenv("ProgramFiles"), Bgetenv("ProgramFiles(x86)") };
//
//    for (i = 0; i < 2; ++i)
//    {
//        if (ProgramFiles[i])
//        {
//            char *buffer = (char*)Bmalloc((strlen(ProgramFiles[i])+1+strlen(p)+1)*sizeof(char));
//            Bsprintf(buffer,"%s/%s",ProgramFiles[i],p);
//            if (addsearchpath(buffer) == 0) // if any work, return success
//                returncode = 0;
//            Bfree(buffer);
//        }
//    }
//
//    return returncode;
	return -1;
}

int32_t win_buildargs(char **argvbuf)
{
    int32_t buildargc = 0;

    *argvbuf = Bstrdup(GetCommandLineA());

    if (*argvbuf)
    {
        char quoted = 0, instring = 0, swallownext = 0;
        char *wp;
        for (const char *p = wp = *argvbuf; *p; p++)
        {
            if (*p == ' ')
            {
                if (instring)
                {
                    if (!quoted)
                    {
                        // end of a string
                        *(wp++) = 0;
                        instring = 0;
                    }
                    else
                        *(wp++) = *p;
                }
            }
            else if (*p == '"' && !swallownext)
            {
                if (instring)
                {
                    if (quoted && p[1] == ' ')
                    {
                        // end of a string
                        *(wp++) = 0;
                        instring = 0;
                    }
                    quoted = !quoted;
                }
                else
                {
                    instring = 1;
                    quoted = 1;
                    buildargc++;
                }
            }
            else if (*p == '\\' && p[1] == '"' && !swallownext)
                swallownext = 1;
            else
            {
                if (!instring)
                    buildargc++;

                instring = 1;
                *(wp++) = *p;
                swallownext = 0;
            }
        }
        *wp = 0;
    }

    return buildargc;
}


// Workaround for a bug in mingwrt-4.0.0 and up where a function named main() in misc/src/libcrt/gdtoa/qnan.c takes precedence over the proper one in src/libcrt/crt/main.c.
#if (defined __MINGW32__ && EDUKE32_GCC_PREREQ(4,8)) || EDUKE32_CLANG_PREREQ(3,4)
# undef main
# include "mingw_main.c"
#endif
