// PolymerNG_Board.h
//

#pragma once

class PolymerNGVisibilityEngine;

//
// PolymerNGBoard
//
class PolymerNGBoard
{
public:
	PolymerNGBoard();
	~PolymerNGBoard();

	void		 DrawRooms(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum);
	void		 CreateProjectionMatrix(int32_t fov, Math::Matrix4 &projectionMatrix, int width, int height);
	void		 AddRenderPlaneToDrawList(BuildRenderThreadTaskRenderWorld &renderWorldTask, Build3DPlane *plane);

	PolymerNGLightLocal		*AddLightToMap(PolymerNGLightLocal light) { mapLights.push_back(light); return &mapLights[mapLights.size() - 1]; }

	Build3DBoard *GetBoard() { return board; }

private:
	void		 InitBoard();
	void		 PokeSector(int16_t secnum);
	void		 FindVisibleSectors(BuildRenderThreadTaskRenderWorld &renderWorldTask, const Math::Matrix4 &modelViewProjectionMatrix, const Math::Matrix4 &modelViewMatrix, const Math::Matrix4 &projectionMatrix, int16_t dacursectnum);
	void		 ScanSprites(int16_t sectnum, tspritetype* localtsprite, int32_t* localspritesortcnt);
	void		 RenderOccluderFromPlane(const Math::Matrix4 &modelViewProjectionMatrix, const Build3DPlane *plane);
	void		 InitOcclusion();
	bool		 IsSectorVisible(const Math::Matrix4 &modelViewProjectionMatrix, Build3DSector *sector);

	void		 DrawSprites(Math::Matrix4 &viewMatrix, Math::Matrix4 &projectionMatrix, float horizang, int16_t daang);
	bool		 ComputeSpritePlane(Math::Matrix4 &viewMatrix, Math::Matrix4 &projectionMatrix, float horizang, int16_t daang, Build3DSprite *sprite, tspritetype *tspr);
	bool		 ComputeModelSpriteRender(Math::Matrix4 &viewMatrix, Math::Matrix4 &projectionMatrix, float horizang, int16_t daang, Build3DSprite *sprite, tspritetype *tspr);

	void		 FindMapSky();

	void		 FindVisibleLightsForScene(PolymerNGLightLocal **lights, int &numVisibleLights);

	Build3DBoard *board;

	PolymerNGMaterial   *boardSkyMaterial;
private:
	Build3DSprite	prsprites[2][MAXSPRITESONSCREEN];

	int32_t         localspritesortcnt;
	float			curskyangmul;
	tspritetype     localtsprite[MAXSPRITESONSCREEN];

	PolymerNGVisibilityEngine *visibilityEngine;
	std::vector<PolymerNGLightLocal> mapLights;
};
