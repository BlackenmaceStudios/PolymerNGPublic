// build3d.h
//

#pragma once

class BuildImage;

#define BUILD3D_INLINE __forceinline
#include <windows.h>
typedef unsigned char byte;

#include "build.h"

//#include "../../rhi/D3D12/Core/PCH.h"
#include <DirectXMath.h>
#include "../src/VectorMath.h"

class BaseModel;
class Build3DBoard;
struct Build3DPlane;
struct Build3DSprite;
struct ModelUpdateQueuedItem;

//
// BuildRenderTaskId
//
enum BuildRenderTaskId
{
	BUILDRENDER_TASK_NOTSET = 0,
	BUILDRENDER_TASK_ROTATESPRITE,
	BUILDRENDER_TASK_CREATEMODEL,
	BUILDRENDER_TASK_RENDERWORLD,
	BUILDRENDER_TASK_DRAWSPRITES,
	BUILDRENDER_TASK_UPDATEMODEL
};

//
// BuildVertex
//
struct BuildVertex
{
	Math::Vector4			vertex;
	Math::Vector3			textureCoords0;
	Math::Vector3			tangent;
	Math::Vector3			binormal;
	Math::Vector3			normal;
};

//
// BuildRenderThreadTaskRenderWorld
//
struct BuildRenderThreadTaskRenderWorld
{
	Math::Vector3			position;
	Math::XMFLOAT4X4		viewProjMatrix;
	Math::XMFLOAT4X4		skyProjMatrix;
	const Build3DPlane		*renderplanes[60000];
	int						numRenderPlanes;
	const Build3DBoard		*board;
	int						gameSmpFrame;
	void					*skyImageHandle;
};

//
// BuildRenderThreadTaskRenderSprites
//
struct BuildRenderThreadTaskRenderSprites
{
	BuildRenderThreadTaskRenderSprites()
	{
		numSprites = 0;
	}

	int					    numSprites;
	Build3DSprite			*prsprites;
};

class BuildRHIMesh;

//
// BuildRenderThreadTaskUpdateModel
//
struct BuildRenderThreadTaskUpdateModel
{
	BaseModel *model;
	BuildRHIMesh *rhiMesh;
	std::vector<ModelUpdateQueuedItem> modelUpdateQueuedItems;
};

//
// BuildRenderThreadTaskCreateModel
//
struct BuildRenderThreadTaskCreateModel
{
	BaseModel *model;
	int startVertex;
	int numVertexes;
	bool createDynamicBuffers;
};

//
// BuildRenderThreadTaskRotateSprite
//
struct BuildRenderThreadTaskRotateSprite
{
	BuildRenderThreadTaskRotateSprite();

	bool is2D;

	bool enableBlend;
	bool enableAlpha;

	float alphaBlendType;


	unsigned int texnum;

	bool drawpoly_srepeat;
	bool drawpoly_trepeat;

	float textureScale_X;
	float textureScale_Y;

	Math::Vector4 spriteColor;

	BuildVertex	vertexes[4];
};

//
// BuildRenderThreadTaskRotateSprite::BuildRenderThreadTaskRotateSprite
//
__forceinline BuildRenderThreadTaskRotateSprite::BuildRenderThreadTaskRotateSprite()
{
	is2D = false;
	enableBlend = false;
	enableAlpha = false;
	alphaBlendType = 0.0f;
	texnum = 0;
	drawpoly_srepeat = false;
	drawpoly_trepeat = false;
	textureScale_X = 1;
	textureScale_Y = 1;
	spriteColor = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

//
// BuildRenderCommand
//
struct BuildRenderCommand
{
	BuildRenderCommand();

	BuildRenderTaskId taskId;
	BuildRenderThreadTaskRotateSprite		taskRotateSprite;
	BuildRenderThreadTaskCreateModel		taskCreateModel;
	BuildRenderThreadTaskRenderWorld		taskRenderWorld;
	BuildRenderThreadTaskRenderSprites		taskRenderSprites;
	BuildRenderThreadTaskUpdateModel		taskUpdateModel;
};

//
// BuildRenderCommand::BuildRenderCommand
//
BUILD3D_INLINE BuildRenderCommand::BuildRenderCommand()
{
	taskId = BUILDRENDER_TASK_NOTSET;
}

/*
==============================

Build3D Map Geo - this is just api agnostic versions of the same stuff inside of Polymer

==============================
*/

//
// Build3DVector4
//
struct Build3DVector4
{
	Build3DVector4() { }
	Build3DVector4(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }
	float x;
	float y;
	float z;
	float w;
};

//
// Build3DVertex
//
struct Build3DVertex
{
	Build3DVertex()
	{
		memset(this, 0, sizeof(Build3DVertex));
	}

	void SetVertex(Build3DVector4 position, Build3DVector4 uv) { this->position = position; this->uv = uv; }

	Build3DVector4 position;
	Build3DVector4 uv;
	Build3DVector4 tangent;
	Build3DVector4 binormal;
	Build3DVector4 normal;
};

//
// Build3DPlane
//
struct Build3DPlane 
{
	Build3DPlane() 
	{
		diffusemodulation[0] = 255; 
		diffusemodulation[1] = 255;
		diffusemodulation[2] = 255;
		diffusemodulation[3] = 255;
		vbo_offset = -1;
		ibo_offset = -1;
		renderImageHandle = NULL;
		isDynamicPlane = false;
		buffer = NULL;
		indices = NULL;
		indicescount = 0;
	}
	// geometry
	Build3DVertex*        buffer;
	int32_t				vertcount;

	// attributes
	float				tbn[3][3];
	float		        plane[4];

	byte				diffusemodulation[4];
	
	int					tileNum;
	void				*renderImageHandle;

	int					vbo_offset;
	int					ibo_offset;

	bool				isDynamicPlane;

	// elements
	unsigned short*     indices;
	int32_t				indicescount;
};

//
// Build3DSector
//
struct Build3DSector 
{
	Build3DSector() {
		memset(this, 0, sizeof(Build3DSector));
		floor.vbo_offset = -1;
		floor.ibo_offset = -1;

		ceil.vbo_offset = -1;
		ceil.ibo_offset = -1;
	}

	const bool			IsCeilParalaxed() const {
		return ceilingstat & 1;
	}

	const bool			IsFloorParalaxed() const {
		return floorstat & 1;
	}

	// polymer data
	double*				 verts;
	Build3DPlane        floor;
	Build3DPlane        ceil;
	int16_t         curindice;
	int32_t         indicescount;
	int32_t         oldindicescount;
	// stuff
	float           wallsproffset;
	float           floorsproffset;
	// build sector data
	int32_t         ceilingz, floorz;
	uint16_t        ceilingstat, floorstat;
	int16_t         ceilingpicnum, ceilingheinum;
	int8_t          ceilingshade;
	uint8_t         ceilingpal, ceilingxpanning, ceilingypanning;
	int16_t         floorpicnum, floorheinum;
	int8_t          floorshade;
	uint8_t         floorpal, floorxpanning, floorypanning;
	uint8_t         visibility;

	int16_t         floorpicnum_anim, ceilingpicnum_anim;

	struct {
		int32_t     empty;
		int32_t     uptodate ;
		int32_t     invalidtex;
	}               flags;
	uint32_t        invalidid;

	bool			isInvalid;
};

//
// Build3DWall
//
struct Build3DWall 
{
	Build3DWall()
	{
		bigportal = NULL;
		cstat = 0;
		picnum = 0;
		overpicnum = 0;
		shade = 0;
		pal = 0;
		xrepeat = 0;
		yrepeat = 0;
		xpanning = 0;
		ypanning = 0;
		nwallpicnum = 0;
		nwallcstat = 0;
		nwallxpanning = 0;
		nwallypanning = 0;
		nwallshade = 0;
		picnum_anim = 0;
		overpicnum_anim = 0;
		underover = 0;
		invalidid = 0;
		flags.uptodate = 0;
		flags.empty = 0;
		flags.invalidtex = 0;
	}
	Build3DPlane        wall;
	Build3DPlane        over;
	Build3DPlane        mask;

	const bool			ShouldRenderWall(tsectortype *sector, walltype *mapwall) const;
	const bool			ShouldRenderOverWall(tsectortype *sector, walltype *mapwall) const;

	// stuff
	float*        bigportal;


	// build wall data
	uint16_t        cstat;
	int16_t         picnum, overpicnum;
	int8_t          shade;
	uint8_t         pal, xrepeat, yrepeat, xpanning, ypanning;

	// nextwall data
	int16_t         nwallpicnum, nwallcstat;
	int8_t          nwallxpanning, nwallypanning;
	int8_t          nwallshade;

	int16_t         picnum_anim, overpicnum_anim;

	char            underover;
	uint32_t        invalidid;
	struct {
		int32_t     empty : 1;
		int32_t     uptodate : 1;
		int32_t     invalidtex : 1;
	}               flags;
};

//
// Build3DSprite
//
struct Build3DSprite 
{
	Build3DPlane       plane;
	Math::XMFLOAT4X4   modelViewProjectionMatrix;
	Math::XMFLOAT4X4   modelMatrix;
	uint32_t        hash;
	bool			isHorizsprite;
	int				paletteNum;
};

//
// Build3DMirror
//
struct Build3DMirror 
{
	Build3DPlane        *plane;
	int16_t         sectnum;
	int16_t         wallnum;
};

//
// Build3DBoard
//
class Build3DBoard
{
public:
	Build3DBoard();
	bool  initsector(int16_t sectnum);
	bool  updatesector(int16_t sectnum);
	bool  initwall(int16_t wallnum);
	bool  updatewall(int16_t wallnum);

	class BaseModel		*GetBaseModel() { return model; }
	const class BaseModel	*GetBaseModel() const { return model; }

	const Build3DSector *GetSector(int index) const { return prsectors[index]; }
	Build3DSector *GetSector(int index) { return prsectors[index]; }

	const Build3DWall *GetWall(int index) const { return prwalls[index]; }
	Build3DWall *GetWall(int index) { return prwalls[index]; }
private:
	bool     buildfloor(int16_t sectnum);
	static void	 tesserror(int error);
	static void	 tessedgeflag(int error);
	static void  tessvertex(void* vertex, void* sector);
	void computeplane(Build3DPlane* p);
	float calc_ypancoef(char curypanning, int16_t curpicnum, int32_t dopancor);

	Build3DSector       *prsectors[MAXSECTORS];
	Build3DWall	        *prwalls[MAXWALLS];

	class BaseModel		*model;

	struct GLUtesselator*  prtess;
};

//
// Build3D
//
class Build3D
{
public:
	static void dorotatesprite(BuildRenderCommand &command, int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum, int8_t dashade, char dapalnum, int32_t dastat, uint8_t daalpha, int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2, int32_t uniqid);
private:
	static void drawpoly(BuildRenderThreadTaskRotateSprite	&taskRotateSprite, vec2f_t const * const dpxy, int32_t const n, int32_t method);
};

extern Build3D	build3D;

#undef small