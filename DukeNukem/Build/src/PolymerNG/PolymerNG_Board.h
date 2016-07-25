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
	void		 CreateProjectionMatrix(int32_t fov, float4x4 &projectionMatrix, int width, int height);
	void		 AddRenderPlaneToDrawList(BuildRenderThreadTaskRenderWorld &renderWorldTask, Build3DPlane *plane, int buildTileNum);

	PolymerNGLightLocal		*AddLightToMap(PolymerNGLightLocal *light) { mapLights.push_back(light); return mapLights[mapLights.size() - 1]; }
	void		RemoveLightFromCurrentBoard(PolymerNGLightLocal	*light);

	Build3DBoard *GetBoard() { return board; }

	void		 MoveLightsInSector(int sectorNum, float deltax, float deltay);

	void		 SetAmbientLightForSector(int sectorNum, int ambientNum);

	void		 DrawSprites(BuildRenderCommand &command, float4x4 &viewMatrix, float4x4 &projectionMatrix, float horizang, int16_t daang, float3 &position);
private:
	void		 GetAmbientSectorColor(int ambientColorId, byte *ambientColorArray);
	void		 InitBoard();
	void		 PokeSector(int16_t secnum);
	void		 FindVisibleSectors(BuildRenderThreadTaskRenderWorld &renderWorldTask, const float4x4 &modelViewProjectionMatrix, const float4x4 &modelViewMatrix, const float4x4 &projectionMatrix, int16_t dacursectnum);
	void		 ScanSprites(int16_t sectnum, tspritetype* localtsprite, int32_t* localspritesortcnt);
	void		 RenderOccluderFromPlane(const float4x4 &modelViewProjectionMatrix, const Build3DPlane *plane);
	void		 InitOcclusion();
	bool		 IsSectorVisible(const float4x4 &modelViewProjectionMatrix, Build3DSector *sector);

	bool		 ComputeSpritePlane(float4x4 &viewMatrix, float4x4 &projectionMatrix, float horizang, int16_t daang, Build3DSprite *sprite, tspritetype *tspr);
	bool		 ComputeModelSpriteRender(float4x4 &viewMatrix, float4x4 &projectionMatrix, float horizang, int16_t daang, Build3DSprite *sprite, tspritetype *tspr);

	void		 FindMapSky();

	void		 FindVisibleLightsForScene(Build3DSprite *prsprites, int numSprites, PolymerNGLightLocal **lights, int &numVisibleLights, float4x4 modelViewMatrix);

	Build3DBoard *board;

	PolymerNGMaterial   *boardSkyMaterial;


private:
	Build3DSprite	prsprites[2][MAXSPRITESONSCREEN];

	int32_t         localspritesortcnt;
	float			curskyangmul;
	tspritetype     localtsprite[MAXSPRITESONSCREEN];

	PolymerNGVisibilityEngine *visibilityEngine;
	std::vector<PolymerNGLightLocal *> mapLights;

	int visibleSectorsArray[MAXSECTORS];
	int numVisibleSectors;
};
