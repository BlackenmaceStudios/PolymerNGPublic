// PolymerNG_Board.cpp
//

#include "PolymerNG_local.h"
#include "Renderer/Renderer.h"

#include "../engine_priv.h"

/*
=============
PolymerNGBoard::PolymerNGBoard
=============
*/
PolymerNGBoard::PolymerNGBoard()
{
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
}
/*
=============
PolymerNGBoard::LoadImageDeferred
=============
*/
BuildImage	 *PolymerNGBoard::LoadImageDeferred(int tileNum)
{
	BuildImage *image = polymerNG.GetImage(tileNum);
	if (!image->IsLoaded())
	{
		image->UpdateImage();
	}

	return image;
}

/*
=============
PolymerNGBoard::InitBoard
=============
*/
void PolymerNGBoard::InitBoard()
{
	board = new Build3DBoard();

	initprintf("--------PolymerNGBoard::InitBoard--------\n");
	initprintf("Loading Sectors\n");

	// Load in all the sectors.
	for (int i = 0; i < numsectors; i++)
	{
		board->initsector(i);
		board->updatesector(i);
	}

	initprintf("Loading Walls\n");
	// Load in all the walls.
	for (int i = 0; i < numwalls; i++)
	{
		board->initwall(i);
		board->updatewall(i);
	}

	for (int i = 0; i < numsectors; i++)
	{
		// Draw the sectors.
		Build3DSector *sector = board->GetSector(i);

		sector->ceil.renderImageHandle = LoadImageDeferred(sector->ceil.tileNum);
		sector->floor.renderImageHandle = LoadImageDeferred(sector->floor.tileNum);
	}

	for (int i = 0; i < numwalls; i++)
	{
		// Draw the sectors.
		Build3DWall *wall = board->GetWall(i);

		wall->wall.renderImageHandle = LoadImageDeferred(wall->picnum);
		wall->over.renderImageHandle = LoadImageDeferred(wall->over.tileNum);
	}

	initprintf("Initializing Occlusion Culling Engine...\n");
	InitOcclusion();

	BaseModel *model = board->GetBaseModel();

	// We can't update memory on the device in the game thread, pass it off to the render thread to deal with.
	BuildRenderCommand command;
	command.taskId = BUILDRENDER_TASK_UPDATEMODEL;
	command.taskUpdateModel.model = model;
	command.taskUpdateModel.startVertex = 0;
	command.taskUpdateModel.numVertexes = model->meshVertexes.size();
	renderer.AddRenderCommand(command);

	initprintf("NumSections = %d\n", numsectors);
	initprintf("NumWalls = %d\n", numwalls);
	initprintf("Vertex Count = %d\n", model->meshVertexes.size());
	initprintf("--------Complete--------\n");
}

void PolymerNGBoard::CreateProjectionMatrix(int32_t fov, Math::Matrix4 &projectionMatrix)
{
	float           aspect;
	float fang = (float)fov * atanf((float)viewingrange / 65536.0f) / (PI / 4);

	aspect = (float)(windowx2 - windowx1 + 1) / (float)(windowy2 - windowy1 + 1);
		
	float matrix[16];
	Math::glhPerspectivef2(matrix, fang / (2048.0f / 360.0f), aspect, 0.01f, 100.0f);
	projectionMatrix.SetX(Math::Vector4(matrix[0], matrix[1], matrix[2], matrix[3]));
	projectionMatrix.SetY(Math::Vector4(matrix[4], matrix[5], matrix[6], matrix[7]));
	projectionMatrix.SetZ(Math::Vector4(matrix[8], matrix[9], matrix[10], matrix[11]));
	projectionMatrix.SetW(Math::Vector4(matrix[12], matrix[13], matrix[14], matrix[15]));
}

/*
=============
PolymerNG::DrawRooms
=============
*/
void PolymerNGBoard::DrawRooms(int32_t daposx, int32_t daposy, int32_t daposz, int16_t daang, int32_t dahoriz, int16_t dacursectnum)
{
	float           skyhoriz, ang, tiltang;
	Math::Vector3 position;
	float			horizang;
	Math::Matrix4 viewMatrix;

	// fogcalc needs this
	//gvisibility = ((float)globalvisibility)*FOGSCALE;

	ang = (float)(daang) / (2048.0f / 360.0f);
	horizang = (float)(-getangle(128, dahoriz - 100)) / (2048.0f / 360.0f);
	tiltang = (gtang * 90.0f);

	// hack for parallax skies
	skyhoriz = horizang;
	if (skyhoriz < -180.0f)
		skyhoriz += 360.0f;

	position.SetX((float)daposy);
	position.SetY(-(float)(daposz) / 16.0f);
	position.SetZ(-(float)daposx);

	viewMatrix.Identity();

	viewMatrix.Rotatef(tiltang, 0.0f, 0.0f, -1.0f);
	viewMatrix.Rotatef(skyhoriz, 1.0f, 0.0f, 0.0f);
	viewMatrix.Rotatef(ang, 0.0f, 1.0f, 0.0f);

	Math::Matrix4 scaleMatrix = viewMatrix.MakeScale(Math::Vector3(1.0f / 1000.0f, 1.0f / 1000.0f, 1.0f / 1000.0f));
	viewMatrix = viewMatrix * scaleMatrix;

	Math::Matrix4 translationMatrix = viewMatrix.MakeTranslation(-position.GetX(), -position.GetY(), -position.GetZ());
	viewMatrix = viewMatrix * translationMatrix;

	Math::Matrix4 projectionMatrix;

	CreateProjectionMatrix(426, projectionMatrix);
	//projectionMatrix.SetX(Math::Vector4(fydimen, 0.0f   , 1.0f, 0.0f));
	//projectionMatrix.SetY(Math::Vector4(0.0f   , fxdimen, 1.0f, 1.0f));
	//projectionMatrix.SetZ(Math::Vector4(0.0f   , 0.0f   , 1.0f, fydimen));
	//projectionMatrix.SetW(Math::Vector4(0.0f   , 0.0f   , -1.0f, 0.0f));

	Math::Matrix4 modelViewProjection = projectionMatrix * viewMatrix;

	BuildRenderCommand command;
	command.taskId = BUILDRENDER_TASK_RENDERWORLD;
	command.taskRenderWorld.board = board;
	command.taskRenderWorld.position = position;
	FindVisibleSectors(command.taskRenderWorld, modelViewProjection, dacursectnum);

	modelViewProjection.GetFloat4x4(&command.taskRenderWorld.viewProjMatrix);

	renderer.AddRenderCommand(command);

	// Now render all the sprites.
	DrawSprites(viewMatrix, projectionMatrix, horizang, daang);
}

/*
=============
PolymerNG::ComputeSpritePlane
=============
*/
void PolymerNGBoard::ComputeSpritePlane(Math::Matrix4 &viewMatrix, Math::Matrix4 &projectionMatrix, float horizang, int16_t daang, Build3DSprite *sprite, tspritetype *tspr)
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

	if (tspr->owner < 0 || tspr->picnum < 0) return;

	if ((tspr->cstat & 8192) )
		return;

	if ((tspr->cstat & 16384))
		return;

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
	tilexoff += (usehightile && h_xsize[curpicnum]) ? h_xoffs[curpicnum] : picanm[curpicnum].xofs;
	tileyoff += (usehightile && h_xsize[curpicnum]) ? h_yoffs[curpicnum] : picanm[curpicnum].yofs;

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

	Math::Matrix4 modelMatrix;

	modelMatrix.Identity();
	sprite->isHorizsprite = false;

	switch (tspr->cstat & SPR_ALIGN_MASK)
	{
	case 0:
		ang = (float)((viewangle)& 2047) / (2048.0f / 360.0f);

		modelMatrix = modelMatrix * Math::Matrix4::MakeTranslation(spos[0], spos[1], spos[2]);
		modelMatrix.Rotatef(-ang, 0.0f, 1.0f, 0.0f);
		modelMatrix.Rotatef(-horizang, 1.0f, 0.0f, 0.0f);
		modelMatrix = modelMatrix * Math::Matrix4::MakeTranslation((float)(-xoff), (float)(yoff), 0.0f);
		modelMatrix = modelMatrix * Math::Matrix4::MakeScale(Math::Vector3((float)(xsize), (float)(ysize), 1.0f));
		break;
	case SPR_WALL:
		ang = (float)((tspr->ang + 1024) & 2047) / (2048.0f / 360.0f);

		modelMatrix = modelMatrix * Math::Matrix4::MakeTranslation(spos[0], spos[1], spos[2]);
		modelMatrix.Rotatef(-ang, 0.0f, 1.0f, 0.0f);
		modelMatrix = modelMatrix * Math::Matrix4::MakeTranslation((float)(-xoff), (float)(yoff - centeryoff), 0.0f);
		modelMatrix = modelMatrix * Math::Matrix4::MakeScale(Math::Vector3((float)(xsize), (float)(ysize), 1.0f));
		
		break;
	case SPR_FLOOR:
		ang = (float)((tspr->ang + 1024) & 2047) / (2048.0f / 360.0f);

		modelMatrix = modelMatrix * Math::Matrix4::MakeTranslation(spos[0], spos[1], spos[2]);
		modelMatrix.Rotatef(-ang, 0.0f, 1.0f, 0.0f);
		modelMatrix = modelMatrix * Math::Matrix4::MakeTranslation((float)(-xoff), 1.0f, (float)(yoff));
		modelMatrix = modelMatrix * Math::Matrix4::MakeScale(Math::Vector3((float)(xsize), 1.0f, (float)(ysize)));

		sprite->isHorizsprite = true;
		break;
	}

	Math::Matrix4 mvp = projectionMatrix * (viewMatrix * modelMatrix);
	mvp.GetFloat4x4(&sprite->modelViewProjectionMatrix);
	modelMatrix.GetFloat4x4(&sprite->modelMatrix);
}

/*
=============
PolymerNG::DrawSprites
=============
*/
void PolymerNGBoard::DrawSprites(Math::Matrix4 &viewMatrix, Math::Matrix4 &projectionMatrix, float horizang, int16_t daang)
{
	BuildRenderCommand command;
	command.taskId = BUILDRENDER_TASK_DRAWSPRITES;
	command.taskRenderSprites.numSprites = localspritesortcnt;
	command.taskRenderSprites.prsprites = &prsprites[0];

	
	for (int i = 0; i < localspritesortcnt; i++)
	{
		tspritetype *tspr = &localtsprite[i];
		Build3DSprite *sprite = &command.taskRenderSprites.prsprites[i];
		int startVertex = i * 4;

		sprite->plane.buffer = NULL;
		sprite->plane.vertcount = 4;
		sprite->plane.vbo_offset = startVertex;

		DO_TILE_ANIM(tspr->picnum, tspr->owner + 32768);

		sprite->plane.tileNum = tspr->picnum;
		sprite->plane.renderImageHandle = LoadImageDeferred(tspr->picnum);
		ComputeSpritePlane(viewMatrix, projectionMatrix, horizang, daang, sprite, tspr);
	}

	renderer.AddRenderCommand(command);
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