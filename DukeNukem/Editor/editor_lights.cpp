#include "pch.h"
#include "build.h"

#include "../Build/src/PolymerNG/PolymerNG_Public.h"
#include "editor_lights.h"

EditorLightSystem editorLightSystem;

EditorLightSystem::EditorLightSystem()
{
	memset(lights, 0, sizeof(PolymerNGLight *) * MAXSPRITES);
}

void EditorLightSystem::SetLightOpts(int spriteNum, PolymerNGLightOpts &opts)
{
	if (lights[spriteNum] == NULL)
	{
		lights[spriteNum] = polymerNGPublic->AddLightToCurrentBoard(opts);
	}
	else
	{
		
	}
}
