// PolymerNG_Board.h
//

#pragma once

#include "PolymerNG_SoftwareRasterizer.h"

//
// PolymerNGBoard
//
class PolymerNGBoard
{
public:
	PolymerNGBoard();
	~PolymerNGBoard();

	void DrawRooms(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum);
	void CreateProjectionMatrix(int32_t fov, Math::Matrix4 &projectionMatrix);
private:
	void		 InitBoard();
	void		 PokeSector(int16_t secnum);
	void		 FindVisibleSectors(BuildRenderThreadTaskRenderWorld &renderWorldTask, const Math::Matrix4 &modelViewProjectionMatrix, int16_t dacursectnum);
	void		 ScanSprites(int16_t sectnum, tspritetype* localtsprite, int32_t* localspritesortcnt);
	void		 RenderOccluderFromPlane(const Math::Matrix4 &modelViewProjectionMatrix, const Build3DPlane *plane);
	void		 InitOcclusion();
	void		 AddRenderPlaneToDrawList(BuildRenderThreadTaskRenderWorld &renderWorldTask, Build3DPlane *plane);
	bool		 IsSectorVisible(const Math::Matrix4 &modelViewProjectionMatrix, Build3DSector *sector);
	BuildImage	 *LoadImageDeferred(int tileNum);

	void		 DrawSprites(Math::Matrix4 &viewMatrix, Math::Matrix4 &projectionMatrix, float horizang, int16_t daang);
	bool		 ComputeSpritePlane(Math::Matrix4 &viewMatrix, Math::Matrix4 &projectionMatrix, float horizang, int16_t daang, Build3DSprite *sprite, tspritetype *tspr);

	void		 FindMapSky();

	Build3DBoard *board;

	BuildImage   *boardSkyImage;
private:
	Build3DSprite	prsprites[MAXSPRITESONSCREEN];

	int32_t         localspritesortcnt;
	float			curskyangmul;
	tspritetype     localtsprite[MAXSPRITESONSCREEN];

	PolymerNGSoftwarRasterizer *softwareRasterizer;
};
