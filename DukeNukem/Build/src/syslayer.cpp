// syslayer.cpp
//

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

#include "PolymerNG/PolymerNG.h"
#include "Input/InputSystem.h"

//#include "../../rhi/D3D12/Core/pch.h"
//#include "../../rhi/D3D12/Core/GameCore.h"

#include "PolymerNG/Renderer/Renderer.h"

#include "Threading/Thread.h"

#include "BuildEngineApp.h"

#include "../Editor/editor_manager.h"

int32_t editor_main(int32_t argc, const char **argv);

class BuildEngineApp : public IBuildEngineApp
{
public:
	// This function can be used to initialize application state and will run after essential
	// hardware resources are allocated.  Some state that does not depend on these resources
	// should still be initialized in the constructor such as pointers and flags.
	virtual void Startup(void);
	virtual void Cleanup(void);

	// The update method will be invoked once per frame.  Both state updating and scene
	// rendering should be handled by this method.
	virtual void Update(float deltaT);

	// Official rendering pass
	virtual void RenderScene(void);

	virtual bool HasWork();

	// Optional UI (overlay) rendering pass.  This is LDR.  The buffer is already cleared.
	virtual void RenderUI(class GraphicsContext& Context);
private:
	int32_t   _buildargc;
	const char **_buildargv;
	char *argvbuf;
	class GameThread *gameThread;
	class EditorThread *editorThread;
};

BuildEngineApp app_local;
IBuildEngineApp *app = &app_local;

class EditorThread : BuildThread
{
public:
	EditorThread(int32_t buildargc, const char **buildargv) : BuildThread(2) {
		_buildargc = buildargc; _buildargv = buildargv;
	}
	virtual int Execute();

	int32_t _buildargc;
	const char **_buildargv;
};

int EditorThread::Execute()
{
	editor_main(_buildargc, _buildargv);

	return 0;
}

class GameThread : BuildThread
{
public:
	GameThread(int32_t buildargc, const char **buildargv) : BuildThread(2) {
		_buildargc = buildargc; _buildargv = buildargv;
	}

	virtual int Execute();

	int32_t _buildargc;
	const char **_buildargv;
};

int GameThread::Execute()
{
	//startwin_open();
	baselayer_init();

	app_main(_buildargc, _buildargv);

	return 0;
}

void BuildEngineApp::Startup()
{
	_buildargc = 0;

	// carve up the command line into more recognizable pieces
	argvbuf = Bstrdup(GetCommandLineA());
	_buildargc = 0;
	if (argvbuf)
	{
		char quoted = 0, instring = 0, swallownext = 0;
		char *p, *wp; int32_t i;
		for (p = wp = argvbuf; *p; p++)
		{
			if (*p == ' ')
			{
				if (instring && !quoted)
				{
					// end of a string
					*(wp++) = 0;
					instring = 0;
				}
				else if (instring)
				{
					*(wp++) = *p;
				}
			}
			else if (*p == '"' && !swallownext)
			{
				if (instring && quoted)
				{
					// end of a string
					if (p[1] == ' ')
					{
						*(wp++) = 0;
						instring = 0;
						quoted = 0;
					}
					else
					{
						quoted = 0;
					}
				}
				else if (instring && !quoted)
				{
					quoted = 1;
				}
				else if (!instring)
				{
					instring = 1;
					quoted = 1;
					_buildargc++;
				}
			}
			else if (*p == '\\' && p[1] == '"' && !swallownext)
			{
				swallownext = 1;
			}
			else
			{
				if (!instring) _buildargc++;
				instring = 1;
				*(wp++) = *p;
				swallownext = 0;
			}
		}
		*wp = 0;

		_buildargv = (const char **)Bmalloc(sizeof(char *)*(_buildargc + 1));
		wp = argvbuf;
		for (i = 0; i < _buildargc; i++, wp++)
		{
			_buildargv[i] = wp;
			while (*wp) wp++;
		}
		_buildargv[_buildargc] = NULL;
	}

	polymerNG.Init();

	numpages = 1;

	if (editorManager.IsEditorModeEnabled())
	{
		editorThread = new EditorThread(_buildargc, _buildargv);
	}
	else
	{
		gameThread = new GameThread(_buildargc, _buildargv);
	}
}

void BuildEngineApp::Cleanup()
{

}

void BuildEngineApp::Update(float deltaT)
{
	xBuildInputSystem->Update();
}

bool BuildEngineApp::HasWork()
{
	return renderer.HasWork();
}

void BuildEngineApp::RenderScene()
{
	renderer.RenderFrame();
}

void BuildEngineApp::RenderUI(class GraphicsContext& Context)
{
	renderer.RenderFrame2D(Context);
}

//
// initprintf() -- prints a formatted string to the intitialization window
//
void initprintf(const char *f, ...)
{
	va_list va;
	char buf[2048];

	va_start(va, f);
	Bvsnprintf(buf, sizeof(buf), f, va);
	va_end(va);

#ifdef _WIN32
	OutputDebugStringA(buf);
#endif

	initputs(buf);
}

//
// wm_setapptitle() -- changes the window title
//
void wm_setapptitle(const char *name)
{

}

// Needed in build.
HWND win_gethwnd(void)
{
	return NULL;
}

//
// win_gethinstance() -- gets the application instance
//
HINSTANCE win_gethinstance(void)
{
	return NULL;
}
