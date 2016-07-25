// PolymerNG_Board.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

#include "../engine_priv.h"
#include "Models/Models.h"

/*
=============
PolymerNGBoard::PolymerNGBoard
=============
*/
PolymerNGBoard::PolymerNGBoard()
{
	visibilityEngine = new PolymerNGVisibilityEngine(this);
	InitBoard();
}

/*
=============
PolymerNGBoard::~PolymerNGBoard
=============
*/
PolymerNGBoard::~PolymerNGBoard()
{
	if (board)
	{
		delete board;
	}

	if (visibilityEngine)
	{
		delete visibilityEngine;
	}
}

/*
=============
PolymerNGBoard::FindMapSky
=============
*/
void PolymerNGBoard::FindMapSky()
{
	int16_t cursky;
	#define DEFAULT_ARTSKY_ANGDIV 4.3027f

	// This is the same logic as in polymer, this can fix easily enough but for now I'm going to leave as is.
	for (int i = 0; i < numsectors; i++)
	{
		if (sector[i].ceilingstat & 1)
		{
			int32_t horizfrac;
			
			//curskypal = sector[i].ceilingpal;
			//cursky = sector[i].ceilingpicnum;
			//curskyshade = sector[i].ceilingshade;
			cursky = sector[i].ceilingpicnum;
			if (forceSkyImage != -1)
				cursky = forceSkyImage;

			getpsky(cursky, &horizfrac, NULL);
			boardSkyMaterial = materialManager.LoadMaterialForTile(cursky);
	
			switch (horizfrac)
			{
			case 0:
				// psky always at same level wrt screen
				curskyangmul = 0.f;
				break;
			case 65536:
				// psky horiz follows camera horiz
				curskyangmul = 1.f;
				break;
			default:
				// sky has hard-coded parallax
				curskyangmul = 1 / DEFAULT_ARTSKY_ANGDIV;
				break;
			}

			return;
		}
	}
}

/*
=============
PolymerNGBoard::InitBoard
=============
*/
void PolymerNGBoard::InitBoard()
{
	imageManager.BeginLevelLoad();
	modelCacheSystem.BeginLevelLoad();
	
	board = new Build3DBoard();

	curskyangmul = 1;

	initprintf("--------PolymerNGBoard::InitBoard--------\n");
	initprintf("Loading Sectors\n");

	// Load in all the sectors.
	for (int i = 0; i < numsectors; i++)
	{
		board->initsector(i);
		board->updatesector(i);
	}

	initprintf("Loading Walls\n");

	for (int i = 0; i < numsectors; i++)
	{
		// Draw the sectors.
		Build3DSector *sector = board->GetSector(i);

		sector->ceil.renderMaterialHandle = materialManager.LoadMaterialForTile(sector->ceil.tileNum);
		sector->floor.renderMaterialHandle = materialManager.LoadMaterialForTile(sector->floor.tileNum);

		//sector->floorstat = ::sector[i].floorstat;
		//sector->ceilingstat = ::sector[i].ceilingstat;
	}

	// Load in all the walls.
	for (int i = 0; i < numwalls; i++)
	{
		board->initwall(i);
		if (!board->updatewall(i))
		{
			initprintf("PolymerNGBoard::InitBoard: Update Wall failed\n");
		}

		Build3DWall *wall = board->GetWall(i);
		wall->wall.renderMaterialHandle = materialManager.LoadMaterialForTile(wall->picnum);

		if (wall->over.buffer && wall->over.tileNum != -1)
		{
			wall->over.renderMaterialHandle = materialManager.LoadMaterialForTile(wall->over.tileNum);
		}

		if (wall->mask.buffer && wall->mask.tileNum != -1)
		{
			wall->mask.renderMaterialHandle = materialManager.LoadMaterialForTile(wall->mask.tileNum);
		}
	}

	// Precache all the models.
	for (int i = 0; i < Numsprites; i++)
	{
		modelCacheSystem.LoadModelForTile(sprite[i].picnum);
	}

	FindMapSky();

	initprintf("Initializing Occlusion Culling Engine...\n");
	InitOcclusion();

	BaseModel *model = board->GetBaseModel();

	// We can't update memory on the device in the game thread, pass it off to the render thread to deal with.
	BuildRenderCommand command;
	command.taskId = BUILDRENDER_TASK_CREATEMODEL;
	command.taskCreateModel.model = model;
	command.taskCreateModel.startVertex = 0;
	command.taskCreateModel.createDynamicBuffers = true;
	command.taskCreateModel.numVertexes = model->meshVertexes.size();
	renderer.AddRenderCommand(command);

	initprintf("NumSections = %d\n", numsectors);
	initprintf("NumWalls = %d\n", numwalls);
	initprintf("Vertex Count = %d\n", model->meshVertexes.size());
	initprintf("--------Complete--------\n");

	modelCacheSystem.EndLevelLoad(model);
	imageManager.EndLevelLoad();
}

void PolymerNGBoard::CreateProjectionMatrix(int32_t fov, float4x4 &projectionMatrix, int width, int height)
{
	float           aspect;
	float fang = (float)fov * atanf((float)viewingrange / 65536.0f) / (PI / 4);

	aspect = (float)(width + 1) / (float)(height + 1);
		
	float matrix[16];
	glhPerspectivef2(matrix, fang / (2048.0f / 360.0f), aspect, 0.01f, 300.0f);
	projectionMatrix.r0 = float4(matrix[0], matrix[1], matrix[2], matrix[3]);
	projectionMatrix.r1 = float4(matrix[4], matrix[5], matrix[6], matrix[7]);
	projectionMatrix.r2 = float4(matrix[8], matrix[9], matrix[10], matrix[11]);
	projectionMatrix.r3 = float4(matrix[12], matrix[13], matrix[14], matrix[15]);
}

/*
=============
PolymerNG::DrawRooms
=============
*/
void PolymerNGBoard::DrawRooms(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum)
{
	float           skyhoriz, ang, tiltang;
	float			horizang;
	float4x4 viewMatrix, rotationMatrix;
	static int currentDrawRoomsIdx = 0;

	// fogcalc needs this
	//gvisibility = ((float)globalvisibility)*FOGSCALE;

	ang = (float)(daang) / (2048.0f / 360.0f);
	horizang = (float)(-getangle(128, dahoriz - 100)) / (2048.0f / 360.0f);
	tiltang = (gtang * 90.0f);

	// hack for parallax skies
	skyhoriz = horizang;
	if (skyhoriz < -180.0f)
		skyhoriz += 360.0f;

	float3	position((float)daposy, -(float)(daposz) / 16.0f, -(float)daposx);

	// if it's not a skybox, make the sky parallax
	// DEFAULT_ARTSKY_ANGDIV is computed from eyeballed values
	// need to recompute it if we ever change the max horiz amplitude
	//skyhoriz *= curskyangmul;

	rotationMatrix = float4x4Identity();

	_math_matrix_rotate(rotationMatrix, tiltang, 0.0f, 0.0f, -1.0f);
	_math_matrix_rotate(rotationMatrix, skyhoriz, 1.0f, 0.0f, 0.0f);
	_math_matrix_rotate(rotationMatrix, ang, 0.0f, 1.0f, 0.0f);

	viewMatrix = rotationMatrix;

	float4x4 skyModelView = viewMatrix;

	float4x4 scaleMatrix = float4x4Scale(1.0f / 1000.0f, 1.0f / 1000.0f, 1.0f / 1000.0f);
	viewMatrix = viewMatrix * scaleMatrix;

	float4x4 translationMatrix = float4x4Translation(-position.x, -position.y, -position.z);
	viewMatrix = viewMatrix * translationMatrix;

	float4x4 projectionMatrix;
	float4x4 occlusionProjectionMatrix;
	CreateProjectionMatrix(426, projectionMatrix, windowx2, windowy2);
	CreateProjectionMatrix(800, occlusionProjectionMatrix, VISPASS_WIDTH, VISPASS_HEIGHT);
	//projectionMatrix.SetX(Math::Vector4(fydimen, 0.0f   , 1.0f, 0.0f));
	//projectionMatrix.SetY(Math::Vector4(0.0f   , fxdimen, 1.0f, 1.0f));
	//projectionMatrix.SetZ(Math::Vector4(0.0f   , 0.0f   , 1.0f, fydimen));
	//projectionMatrix.SetW(Math::Vector4(0.0f   , 0.0f   , -1.0f, 0.0f));

	float4x4 modelViewProjection = projectionMatrix * viewMatrix;

	float4x4 skyModelViewProjection = projectionMatrix * skyModelView;

	float4x4 occlusionViewProjection = occlusionProjectionMatrix * viewMatrix;

	int16_t cursectnum = dacursectnum;
	updatesectorbreadth(daposx, daposy, &cursectnum);

	int smpframe = renderer.GetCurrentFrameNum();
	{
		if (board->GetBaseModel()->dynamicBufferDirty[smpframe])
		{
			BuildRenderCommand command;
			command.taskId = BUILDRENDER_TASK_UPDATEMODEL;
			command.taskUpdateModel.rhiMesh = board->GetBaseModel()->rhiVertexBufferStatic;

			command.taskUpdateModel.model = board->GetBaseModel();
			board->GetBaseModel()->dynamicBufferDirty[smpframe] = false;
			renderer.AddRenderCommand(command);
		}
		
	}



	{
		BuildRenderCommand command;
		command.taskId = BUILDRENDER_TASK_RENDERWORLD;
		command.taskRenderWorld.board = board;
		command.taskRenderWorld.position = position;
		command.taskRenderWorld.skyMaterialHandle = boardSkyMaterial;
		command.taskRenderWorld.gameSmpFrame = smpframe;
		command.taskRenderWorld.renderplanes = command.taskRenderWorld.renderplanesFrames[currentDrawRoomsIdx][smpframe];
		FindVisibleSectors(command.taskRenderWorld, modelViewProjection, viewMatrix, projectionMatrix, cursectnum);

		float4x4 inverseView = viewMatrix;
		inverseView.invert();

		command.taskRenderWorld.inverseViewMatrix = inverseView;
		command.taskRenderWorld.projectionMatrix = projectionMatrix;
		command.taskRenderWorld.viewProjMatrix = modelViewProjection;
		command.taskRenderWorld.viewMatrix = viewMatrix;
		command.taskRenderWorld.skyProjMatrix = skyModelViewProjection;
		command.taskRenderWorld.occlusionViewProjMatrix = occlusionViewProjection;

		renderer.AddRenderCommand(command);
	}

	currentDrawRoomsIdx++;
	if (currentDrawRoomsIdx > 3)
		currentDrawRoomsIdx = 0;

	// Now render all the sprites.
	int					    numSprites;
	Build3DSprite			*prsprites;
	{
		BuildRenderCommand command;
		command.taskId = BUILDRENDER_TASK_DRAWSPRITES;
		DrawSprites(command, viewMatrix, projectionMatrix, horizang, daang, position);
		numSprites = command.taskRenderSprites.numSprites;
		prsprites = command.taskRenderSprites.prsprites;
		renderer.AddRenderCommand(command);
		Bmemcpy(tsprite, localtsprite, sizeof(spritetype) * spritesortcnt);
	}


	// Now draw all the lights.
	{
		float4x4 inverseModelViewProjection = projectionMatrix;
		inverseModelViewProjection.invert();

		float4x4 inverseModelViewInverse = viewMatrix;
		inverseModelViewInverse.invert();

		float4x4 inverseModelViewProjectionMatrix = modelViewProjection;
		inverseModelViewProjectionMatrix.invert();

		BuildRenderCommand command;
		command.taskId = BUILDRENDER_TASK_DRAWLIGHTS;
		command.taskDrawLights.inverseModelViewProjectionMatrix = inverseModelViewProjectionMatrix;
		command.taskDrawLights.inverseModelViewMatrix = inverseModelViewProjection;
		command.taskDrawLights.inverseViewMatrix = inverseModelViewInverse;
		command.taskDrawLights.viewMatrix = viewMatrix;
		command.taskDrawLights.cameraposition = float4(position.x, position.y, position.z, 1.0);

		float4x4 viewMatrix_((float *)&viewMatrix);
		FindVisibleLightsForScene(prsprites, numSprites, &command.taskDrawLights.visibleLights[0], command.taskDrawLights.numLights, viewMatrix_);
		renderer.AddRenderCommand(command);
	}
}

/*
=============
PolymerNG::ComputeSpritePlane
=============
*/
bool PolymerNGBoard::ComputeSpritePlane(float4x4 &viewMatrix, float4x4 &projectionMatrix, float horizang, int16_t daang, Build3DSprite *sprite, tspritetype *tspr)
{
	int32_t         curpicnum, xsize, ysize, i, j;
	int32_t         tilexoff, tileyoff, xoff, yoff, centeryoff = 0;
	float           xratio, yratio, ang, f;
	float           spos[3];
	uint8_t         flipu, flipv;
	int16_t			viewangle = daang;

	curpicnum = tspr->picnum;
	DO_TILE_ANIM(curpicnum, tspr->owner + 32768);

	bool			usehightile = true;

	const uint32_t cs = tspr->cstat;
	const uint32_t alignmask = (cs & SPR_ALIGN_MASK);
	const uint8_t flooraligned = (alignmask == SPR_FLOOR);

	if (tspr->owner < 0 || tspr->picnum < 0) return false;

	if (tspr->sectnum == MAXSECTORS)
		return false;

	//if ((tspr->cstat & 8192) )
	//	return;

	if ((tspr->cstat & 16384) && buildNGOptions.shouldUseHighSpriteValueHide)
		return false;

	if (((tspr->cstat >> 4) & 3) == 0)
		xratio = (float)(tspr->xrepeat) * 0.20f; // 32 / 160
	else
		xratio = (float)(tspr->xrepeat) * 0.25f;

	yratio = (float)(tspr->yrepeat) * 0.25f;

	xsize = tilesiz[curpicnum].x;
	ysize = tilesiz[curpicnum].y;

	if (usehightile && h_xsize[curpicnum])
	{
		xsize = h_xsize[curpicnum];
		ysize = h_ysize[curpicnum];
	}

	xsize = (int32_t)(xsize * xratio);
	ysize = (int32_t)(ysize * yratio);

	tilexoff = (int32_t)tspr->xoffset;
	tileyoff = (int32_t)tspr->yoffset;
	tilexoff += (usehightile && h_xsize[curpicnum]) ? h_xoffs[curpicnum] : picanm[curpicnum].flags.xofs;
	tileyoff += (usehightile && h_xsize[curpicnum]) ? h_yoffs[curpicnum] : picanm[curpicnum].flags.yofs;

	xoff = (int32_t)(tilexoff * xratio);
	yoff = (int32_t)(tileyoff * yratio);

	if ((tspr->cstat & 128) && !flooraligned)
	{
		if (alignmask == 0)
			yoff -= ysize / 2;
		else
			centeryoff = ysize / 2;
	}

	spos[0] = (float)tspr->y;
	spos[1] = -(float)(tspr->z) / 16.0f;
	spos[2] = -(float)tspr->x;


	{
		const uint8_t xflip = !!(cs & SPR_XFLIP);
		const uint8_t yflip = !!(cs & SPR_YFLIP);

		// Initially set flipu and flipv.
		flipu = (xflip ^ flooraligned);
		flipv = (yflip && !flooraligned);

		if (alignmask == 0)
		{
			// do surgery on the face tspr to make it look like a wall sprite
			tspr->cstat |= 16;
			tspr->ang = (viewangle + 1024) & 2047;
		}

		if (flipu)
			xoff = -xoff;

		if (yflip && alignmask != 0)
			yoff = -yoff;
	}

	float4x4 modelMatrix;

	modelMatrix = float4x4Identity();
	sprite->isHorizsprite = false;

	switch (tspr->cstat & SPR_ALIGN_MASK)
	{
	case 0:
		ang = (float)((viewangle)& 2047) / (2048.0f / 360.0f);

		modelMatrix = modelMatrix * float4x4Translation(spos[0], spos[1], spos[2]);
		_math_matrix_rotate(modelMatrix,-ang, 0.0f, 1.0f, 0.0f);
		_math_matrix_rotate(modelMatrix,-horizang, 1.0f, 0.0f, 0.0f);
		modelMatrix = modelMatrix * float4x4Translation((float)(-xoff), (float)(yoff), 0.0f);
		modelMatrix = modelMatrix * float4x4Scale((float)(xsize), (float)(ysize), 1.0f);
		break;
	case SPR_WALL:
		ang = (float)((tspr->ang + 1024) & 2047) / (2048.0f / 360.0f);

		modelMatrix = modelMatrix * float4x4Translation(spos[0], spos[1], spos[2]);
		_math_matrix_rotate(modelMatrix,-ang, 0.0f, 1.0f, 0.0f);
		modelMatrix = modelMatrix * float4x4Translation((float)(-xoff), (float)(yoff - centeryoff), 0.0f);
		modelMatrix = modelMatrix * float4x4Scale((float)(xsize), (float)(ysize), 1.0f);
		sprite->isWallSprite = true;
		break;
	case SPR_FLOOR:
		ang = (float)((tspr->ang + 1024) & 2047) / (2048.0f / 360.0f);

		modelMatrix = modelMatrix * float4x4Translation(spos[0], spos[1], spos[2]);
		_math_matrix_rotate(modelMatrix, -ang, 0.0f, 1.0f, 0.0f);
		modelMatrix = modelMatrix * float4x4Translation((float)(-xoff), 1.0f, (float)(yoff));
		modelMatrix = modelMatrix * float4x4Scale((float)(xsize), 1.0f, (float)(ysize));

		sprite->isHorizsprite = true;
		break;
	}

	sprite->plane.visibility = sector[tspr->sectnum].visibility;
	sprite->plane.shadeNum = tspr->shade;
	//Build3D::CalculateFogForPlane(sprite->plane.tileNum, sprite->plane.shadeNum, sprite->plane.visibility, sprite->plane.paletteNum, &sprite->plane);

#if !POLYMERNG_NOSYNC_SPRITES
	float4x4 mvp = projectionMatrix * (viewMatrix * modelMatrix);
#else
	float4x4 mvp = projectionMatrix * (viewMatrix);
#endif
	sprite->modelViewProjectionMatrix = mvp;
	sprite->modelMatrix = modelMatrix;
	sprite->ViewMatrix = viewMatrix;


	float4x4 modelViewInverseMatrix = viewMatrix;
	modelViewInverseMatrix.invert();
	sprite->modelViewInverse = modelViewInverseMatrix;

	return true;
}

//
// PolymerNGBoard::ComputeModelSpriteRender
//
bool PolymerNGBoard::ComputeModelSpriteRender(float4x4 &viewMatrix, float4x4 &projectionMatrix, float horizang, int16_t daang, Build3DSprite *sprite, tspritetype *tspr)
{
	float           *v0, *v1;
	char            targetpal, usinghighpal, foundpalskin;
	float           spos2[3], spos[3], tspos[3], lpos[3], tlpos[3], vec[3], mat[4][4];
	float           ang;
	float           scale;
	double          det;
	int32_t         surfi, i, j;
	int32_t         materialbits;
	float           sradius, lradius;
	char            modellightcount;
	uint8_t         curpriority;

//	uint8_t lpal = (tspr->owner >= MAXSPRITES) ? tspr->pal : sprite[tspr->owner].pal;

	// Hackish, but that means it's a model drawn by rotatesprite.
	if (tspriteptr[MAXSPRITESONSCREEN] == tspr) {
		float       x, y, z;

		spos[0] = (float)globalposy;
		spos[1] = -(float)(globalposz) / 16.0f;
		spos[2] = -(float)globalposx;

		// The coordinates are actually floats disguised as int in this case
		memcpy(&x, &tspr->x, sizeof(float));
		memcpy(&y, &tspr->y, sizeof(float));
		memcpy(&z, &tspr->z, sizeof(float));

		spos2[0] = (float)y - globalposy;
		spos2[1] = -(float)(z - globalposz) / 16.0f;
		spos2[2] = -(float)(x - globalposx);
	}
	else {
		spos[0] = (float)tspr->y;
		spos[1] = -(float)(tspr->z) / 16.0f;
		spos[2] = -(float)tspr->x;

		spos2[0] = spos2[1] = spos2[2] = 0.0f;
	}

	ang = (float)((tspr->ang + spriteext[tspr->owner].angoff) & 2047) / (2048.0f / 360.0f);
	ang -= 90.0f;
	if (((tspr->cstat >> 4) & 3) == 2)
		ang -= 90.0f;

	float mscale = max(modelCacheSystem.GetModelOverridesForId(tspr->picnum)->scale, 1);

	float4x4 modelMatrix;
	modelMatrix = float4x4Identity();
	scale = (1.0 / 4.0);
	scale *= 0.7f;
	scale *= mscale;
//	if (pr_overridemodelscale) {
//		scale *= pr_overridemodelscale;
//	}
//	else {
//		scale *= m->bscale;
//	}

	if (tspriteptr[MAXSPRITESONSCREEN] == tspr) {
		float playerang, radplayerang, cosminusradplayerang, sinminusradplayerang, hudzoom;

		playerang = (globalang & 2047) / (2048.0f / 360.0f) - 90.0f;
		radplayerang = (globalang & 2047) * 2.0f * PI / 2048.0f;
		cosminusradplayerang = cos(-radplayerang);
		sinminusradplayerang = sin(-radplayerang);
		hudzoom = 65536.0 / spriteext[tspr->owner].offset.z;

		modelMatrix = modelMatrix * float4x4Translation(spos[0], spos[1], spos[2]); //bglTranslatef(spos[0], spos[1], spos[2]);
		_math_matrix_rotate(modelMatrix, horizang, -cosminusradplayerang, 0.0f, sinminusradplayerang);
		_math_matrix_rotate(modelMatrix, spriteext[tspr->owner].roll / (2048.0f / 360.0f), sinminusradplayerang, 0.0f, cosminusradplayerang);
		_math_matrix_rotate(modelMatrix, -playerang, 0.0f, 1.0f, 0.0f);

		_math_matrix_rotate(modelMatrix, -playerang, 0.0f, 1.0f, 0.0f);
		modelMatrix = modelMatrix * float4x4Scale(hudzoom, 1.0f, 1.0f);
		_math_matrix_rotate(modelMatrix, playerang, 0.0f, 1.0f, 0.0f);
		modelMatrix = modelMatrix * float4x4Translation(spos2[0], spos2[1], spos2[2]); //bglTranslatef(spos[0], spos[1], spos[2]); bglTranslatef(spos2[0], spos2[1], spos2[2]);
		_math_matrix_rotate(modelMatrix, -ang, 0.0f, 1.0f, 0.0f);
	}
	else {
		modelMatrix = modelMatrix * float4x4Translation(spos[0], spos[1], spos[2]); //bglTranslatef(spos[0], spos[1], spos[2]); bglTranslatef(spos[0], spos[1], spos[2]);
		_math_matrix_rotate(modelMatrix, -ang, 0.0f, 1.0f, 0.0f); // jm
	}
	if (((tspr->cstat >> 4) & 3) == 2)
	{
		modelMatrix = modelMatrix * float4x4Translation(0.0f, 0.0, -(float)(tilesiz[tspr->picnum].y * tspr->yrepeat) / 8.0f);
		_math_matrix_rotate(modelMatrix, 90.0f, 0.0f, 0.0f, 1.0f);
	}
	else
		_math_matrix_rotate(modelMatrix, -90.0f, 1.0f, 0.0f, 0.0f);

	if ((tspr->cstat & 128) && (((tspr->cstat >> 4) & 3) != 2))
		modelMatrix = modelMatrix * float4x4Translation(0.0f, 0.0, -(float)(tilesiz[tspr->picnum].y * tspr->yrepeat) / 8.0f);

	// yoffset differs from zadd in that it does not follow cstat&8 y-flipping
	modelMatrix = modelMatrix * float4x4Translation(0.0f, 0.0, 0 * 64 * scale * tspr->yrepeat); // jm

	if (tspr->cstat & 8)
	{
		modelMatrix = modelMatrix * float4x4Translation(0.0f, 0.0, (float)(tilesiz[tspr->picnum].y * tspr->yrepeat) / 4.0f);
		modelMatrix = modelMatrix * float4x4Scale(1.0f, 1.0f, -1.0f);
	}

	if (tspr->cstat & 4)
		modelMatrix = modelMatrix * float4x4Scale(1.0f, -1.0f, 1.0f);

	//if (!(tspr->cstat & 4) != !(tspr->cstat & 8)) {
	//	// Only inverting one coordinate will reverse the winding order of
	//	// faces, so we need to account for that when culling.
	//	SWITCH_CULL_DIRECTION;
	//}

	// jmarshall
	modelMatrix = modelMatrix * float4x4Scale(scale * tspr->xrepeat, scale * tspr->xrepeat, scale * tspr->yrepeat);
	modelMatrix = modelMatrix * float4x4Translation(0.0f, 0.0, 0 * 64);
	// jmarshall end

	// scripted model rotation
	// jmarshall
	//if (tspr->owner < MAXSPRITES &&
	//	(spriteext[tspr->owner].pitch || spriteext[tspr->owner].roll))
	//{
	//	float       pitchang, rollang, offsets[3];
	//
	//	pitchang = (float)(spriteext[tspr->owner].pitch) / (2048.0f / 360.0f);
	//	rollang = (float)(spriteext[tspr->owner].roll) / (2048.0f / 360.0f);
	//
	//	offsets[0] = -spriteext[tspr->owner].offset.x / (scale * tspr->xrepeat);
	//	offsets[1] = -spriteext[tspr->owner].offset.y / (scale * tspr->xrepeat);
	//	offsets[2] = (float)(spriteext[tspr->owner].offset.z) / 16.0f / (scale * tspr->yrepeat);
	//
	//	modelMatrix = modelMatrix * float4x4::MakeTranslation(-offsets[0], -offsets[1], -offsets[2]);
	//
	//	modelMatrix.Rotatef(pitchang, 0.0f, 1.0f, 0.0f);
	//	modelMatrix.Rotatef(rollang, -1.0f, 0.0f, 0.0f);
	//
	//	modelMatrix = modelMatrix * float4x4::MakeTranslation(offsets[0], offsets[1], offsets[2]);
	//}
	// jmarshall end

	//bglGetFloatv(GL_MODELVIEW_MATRIX, spritemodelview);
	//
	//bglPopMatrix();
	//bglPushMatrix();
	//bglMultMatrixf(spritemodelview);
	//
	//// invert this matrix to get the polymer -> mdsprite space
	//memcpy(mat, spritemodelview, sizeof(float) * 16);
	//INVERT_4X4(mdspritespace, det, mat);

	// debug code for drawing the model bounding sphere
	//     bglDisable(GL_TEXTURE_2D);
	//     bglBegin(GL_LINES);
	//     bglColor4f(1.0, 0.0, 0.0, 1.0);
	//     bglVertex3f(m->head.frames[m->cframe].cen.x,
	//                 m->head.frames[m->cframe].cen.y,
	//                 m->head.frames[m->cframe].cen.z);
	//     bglVertex3f(m->head.frames[m->cframe].cen.x + m->head.frames[m->cframe].r,
	//                 m->head.frames[m->cframe].cen.y,
	//                 m->head.frames[m->cframe].cen.z);
	//     bglColor4f(0.0, 1.0, 0.0, 1.0);
	//     bglVertex3f(m->head.frames[m->cframe].cen.x,
	//                 m->head.frames[m->cframe].cen.y,
	//                 m->head.frames[m->cframe].cen.z);
	//     bglVertex3f(m->head.frames[m->cframe].cen.x,
	//                 m->head.frames[m->cframe].cen.y + m->head.frames[m->cframe].r,
	//                 m->head.frames[m->cframe].cen.z);
	//     bglColor4f(0.0, 0.0, 1.0, 1.0);
	//     bglVertex3f(m->head.frames[m->cframe].cen.x,
	//                 m->head.frames[m->cframe].cen.y,
	//                 m->head.frames[m->cframe].cen.z);
	//     bglVertex3f(m->head.frames[m->cframe].cen.x,
	//                 m->head.frames[m->cframe].cen.y,
	//                 m->head.frames[m->cframe].cen.z + m->head.frames[m->cframe].r);
	//     bglEnd();
	//     bglEnable(GL_TEXTURE_2D);

//	polymer_getscratchmaterial(&mdspritematerial);
//
//	color = mdspritematerial.diffusemodulation;
//
//	color[0] = color[1] = color[2] =
//		(GLubyte)(((float)(numshades - min(max((tspr->shade * shadescale) + m->shadeoff, 0), numshades))) / ((float)numshades) * 0xFF);
//
//	usinghighpal = (pr_highpalookups &&
//		prhighpalookups[curbasepal][tspr->pal].map);
//
//	// tinting
//	if (!usinghighpal && !(hictinting[tspr->pal].f & HICTINT_PRECOMPUTED))
//	{
//		if (!(m->flags & 1))
//			hictinting_apply_ub(color, tspr->pal);
//		else globalnoeffect = 1; //mdloadskin reads this
//	}
//
//	// global tinting
//	if (!usinghighpal && have_basepal_tint())
//		hictinting_apply_ub(color, MAXPALOOKUPS - 1);
//
//	if (tspr->cstat & 2)
//	{
//		if (!(tspr->cstat & 512))
//			color[3] = 0xAA;
//		else
//			color[3] = 0x55;
//	}
//	else
//		color[3] = 0xFF;
//
//	{
//		double f = color[3] * (1.0f - spriteext[tspr->owner].alpha);
//		color[3] = (GLubyte)f;
//	}

	modellightcount = 0;
	curpriority = 0;

	sprite->plane.visibility = sector[tspr->sectnum].visibility;
	sprite->plane.shadeNum = tspr->shade;
	//Build3D::CalculateFogForPlane(sprite->plane.tileNum, sprite->plane.shadeNum, sprite->plane.visibility, sprite->plane.paletteNum, &sprite->plane);

	float4x4 modelViewMatrix = (viewMatrix * modelMatrix);
	float4x4 mvp = projectionMatrix * modelViewMatrix;
	sprite->modelViewProjectionMatrix = mvp;
	sprite->modelMatrix = modelMatrix;

	return true;
}

/*
=============
PolymerNG::RemoveLightFromCurrentBoard
=============
*/
void PolymerNGBoard::RemoveLightFromCurrentBoard(PolymerNGLightLocal *light)
{
	for (int i = 0; i < mapLights.size(); i++)
	{
		if (mapLights[i] == light)
		{
			mapLights.erase(mapLights.begin() + i);
			return;
		}
	}

	initprintf("PolymerNGBoard::RemoveLightFromCurrentBoard: Failed to remove light\n");
}

/*
=============
PolymerNG::GetAmbientSectorColor
=============
*/
void PolymerNGBoard::GetAmbientSectorColor(int ambientColorId, byte *ambientColorArray)
{
	switch (ambientColorId)
	{
		case 1:
			ambientColorArray[0] = 6;
			ambientColorArray[1] = 6;
			ambientColorArray[2] = 40;
			break;

		case 2:
			ambientColorArray[0] = 5;
			ambientColorArray[1] = 5;
			ambientColorArray[2] = 5;
			break;
	}
}

/*
=============
PolymerNG::DrawSprites
=============
*/
void PolymerNGBoard::DrawSprites(BuildRenderCommand &command, float4x4 &viewMatrix, float4x4 &projectionMatrix, float horizang, int16_t daang, float3 &position)
{
	command.taskRenderSprites.numSprites = localspritesortcnt;
	command.taskRenderSprites.prsprites = &prsprites[renderer.GetCurrentFrameNum()][0];
	command.taskRenderSprites.position = position;
	
	for (int i = 0; i < localspritesortcnt; i++)
	{
		tspritetype *tspr = &tsprite[i];
		Build3DSprite *sprite = &command.taskRenderSprites.prsprites[i];
		int startVertex = i * 4;

		// Load in the model for this sprite, if its preached this should NOT cause any hitches...
		sprite->cacheModel = modelCacheSystem.LoadModelForTile(tspr->picnum);

		sprite->plane.buffer = NULL;
		sprite->plane.vertcount = 4;
		sprite->plane.vbo_offset = startVertex;

		if (!sprite->cacheModel)
		{
			DO_TILE_ANIM(tspr->picnum, tspr->owner + 32768);
		}
		
		sprite->paletteNum = tspr->pal;
		sprite->plane.tileNum = tspr->picnum;
		sprite->plane.renderMaterialHandle = materialManager.LoadMaterialForTile(tspr->picnum);

		if (sprite->cacheModel)
		{
			sprite->isVisible = ComputeModelSpriteRender(viewMatrix, projectionMatrix, horizang, daang, sprite, tspr);
		}
		else
		{
			sprite->isVisible = ComputeSpritePlane(viewMatrix, projectionMatrix, horizang, daang, sprite, tspr);
		}
		
		GetAmbientSectorColor(board->GetSector(tspr->sectnum)->ambientSectorId, sprite->ambientColor);
	}
}

/*
=============
PolymerNG::LoadBoard
=============
*/
void PolymerNG::LoadBoard()
{
	polymerNGPrivate.currentBoard = new PolymerNGBoard();
}

/*
=============
PolymerNG::DrawRooms
=============
*/
void PolymerNG::DrawRooms(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum)
{
	polymerNGPrivate.currentBoard->DrawRooms(daposx, daposy, daposz, daang, dahoriz, dacursectnum);
}

/*
=============
PolymerNG::AddLightToCurrentBoard
=============
*/
PolymerNGLight *PolymerNG::AddLightToCurrentBoard(PolymerNGLightOpts lightOpts)
{
	PolymerNGLightLocal *light = new PolymerNGLightLocal(lightOpts, polymerNGPrivate.currentBoard);
	return polymerNGPrivate.currentBoard->AddLightToMap(light);
}

/*
=============
PolymerNG::RemoveLightFromCurrentBoard
=============
*/
void PolymerNG::RemoveLightFromCurrentBoard(PolymerNGLight *light)
{
	polymerNGPrivate.currentBoard->RemoveLightFromCurrentBoard((PolymerNGLightLocal *)light);
}