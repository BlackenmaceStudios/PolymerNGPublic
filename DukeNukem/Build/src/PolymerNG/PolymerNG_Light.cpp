// PolymerNG_Light.cpp
//

#include "PolymerNG_local.h"

PolymerNGLightLocal::PolymerNGLightLocal(PolymerNGLightOpts opts)
{
	this->opts = opts;
}

//
// PolymerNGBoard::FindVisibleLightsForScene
//
void PolymerNGBoard::FindVisibleLightsForScene(PolymerNGLightLocal **lights, int &numVisibleLights)
{
	int numVisibleLightsInFrame = mapLights.size(); // Todo: Add Culling here!!!!

	if (numVisibleLightsInFrame >= MAX_VISIBLE_LIGHTS)
	{
	//	initprintf("PolymerNGBoard::FindVisibleLightsForScene: Too many lights in view at once!\n");
		return;
	}
	
	for (int i = 0; i < numVisibleLightsInFrame; i++)
	{
		lights[i] = &mapLights[i];
	}
}