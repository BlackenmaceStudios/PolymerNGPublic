// PolymerNG_Visibility.h
//

#pragma once

class PolymerNGBoard;

//
// PolymerNGVisibilityEngine
//
class PolymerNGVisibilityEngine
{
public:
	PolymerNGVisibilityEngine(PolymerNGBoard *board);
	~PolymerNGVisibilityEngine();

	void					CreateOcclusionFromBoard();

	void					FindVisibleSectors(BuildRenderThreadTaskRenderWorld &renderWorldTask, float *modelViewMatrix, float *projectionMatrix, std::vector<int32_t> &visibleSectorList);
private:

	PolymerNGBoard			*currentBoard;
};