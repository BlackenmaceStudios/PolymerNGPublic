// PolymerNG_Visibility
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

#include "../engine_priv.h"
#include "Models/Models.h"

#include <algorithm>

float gOccluderSizeThreshold = 1.5f;
float gOccludeeSizeThreshold = 0.01f;

float *currentModelViewMatrixCulling;

PolymerNGVisibilityEngine::PolymerNGVisibilityEngine(PolymerNGBoard *board)
{
	currentBoard = board;
}

PolymerNGVisibilityEngine::~PolymerNGVisibilityEngine()
{
}

//
// PolymerNGVisibilityEngine::CreateOcclusionFromBoard
//
void PolymerNGVisibilityEngine::CreateOcclusionFromBoard()
{
	// Don't have to anything :).
}

//
// PolymerNGVisibilityEngine::FindVisibleSectors
//
void PolymerNGVisibilityEngine::FindVisibleSectors(BuildRenderThreadTaskRenderWorld &renderWorldTask, float *modelViewMatrix, float *projectionMatrix, std::vector<int32_t> &visibleSectorList)
{
	Build3DBoardPolymost *polymostGenericRenderer = ((Build3DBoard *)renderWorldTask.board)->GetGenericPolymostRenderer();
	polymostGenericRenderer->drawrooms();

	visibleSectorList = polymostGenericRenderer->visibleSectorList;

//	std::sort(visibleSectorList.begin(), visibleSectorList.end());
//	visibleSectorList.erase(std::unique(visibleSectorList.begin(), visibleSectorList.end()), visibleSectorList.end());
}