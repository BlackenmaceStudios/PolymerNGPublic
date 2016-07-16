// editor_manager.cpp
//

#include "pch.h"
#include "editor_manager.h"

EditorManager editorManager;

#ifdef SWGAME
#define NOEDITOR
#endif

#ifdef NOEDITOR
int32_t zoom = 0;
int32_t editor_main(int32_t argc, const char **argv)
{
	return 0;
}
#endif