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
#include "../src/IntelBuildCPUT/CPUTMath.h"
#include "../src/IntelBuildCPUT/AxisAlignedBox.h"

class BaseModel;
class Build3DBoard;
struct Build3DPlane;
struct Build3DSprite;
struct ModelUpdateQueuedItem;

extern float *currentModelViewMatrixCulling;


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
	BUILDRENDER_TASK_UPDATEMODEL,
	BUILDRENDER_TASK_DRAWLIGHTS,
	BUILDRENDER_TASK_DRAWCLASSICSCREEN
};

//
// BuildVertex
//
struct BuildVertex
{
	float4			vertex;
	float3			textureCoords0;
	float3			normal;
};

#define MAX_CONCURRENT_DRAWBOARDS 5

extern const Build3DPlane		*renderPlanesGlobalPool[MAX_CONCURRENT_DRAWBOARDS][60000];
extern const Build3DPlane		*renderPlanesGlobalPool2[MAX_CONCURRENT_DRAWBOARDS][60000];

//
// BuildRenderThreadTaskDrawLights
//
class PolymerNGLightLocal;
#define MAX_VISIBLE_LIGHTS 100
struct BuildRenderThreadTaskDrawLights
{
	PolymerNGLightLocal *visibleLights[MAX_VISIBLE_LIGHTS];
	int numLights;
	float4x4 inverseModelViewMatrix;
	float4x4 inverseModelViewProjectionMatrix;
	float4x4 inverseViewMatrix;
	float4x4 viewMatrix;
	float4 cameraposition;
};

//
// BuildRenderThreadDrawClassicScreen
//
struct BuildRenderThreadDrawClassicScreen
{
	int width;
	int height;
	byte *screen_buffer;
};

//
// BuildRenderThreadTaskRenderWorld
//
struct BuildRenderThreadTaskRenderWorld
{
	BuildRenderThreadTaskRenderWorld()
	{
		skyMaterialHandle = NULL;
		renderplanes = NULL;

		for (int i = 0; i < MAX_CONCURRENT_DRAWBOARDS; i++)
		{
			renderplanesFrames[i][0] = (const Build3DPlane	**)&renderPlanesGlobalPool[i][0]; // We only draw one board at a time.
			renderplanesFrames[i][1] = (const Build3DPlane	**)&renderPlanesGlobalPool2[i][1]; // We only draw one board at a time.
		}
	}
	float3					position;
	float4x4				viewProjMatrix;
	float4x4				viewMatrix;
	float4x4				inverseViewMatrix;
	float4x4				projectionMatrix;
	float4x4				skyProjMatrix;
	float4x4				occlusionViewProjMatrix;
	const Build3DPlane		**renderplanes;
	const Build3DPlane		**renderplanesFrames[MAX_CONCURRENT_DRAWBOARDS][2];
	int						numRenderPlanes;
	const Build3DBoard		*board;
	int						gameSmpFrame;
	void					*skyMaterialHandle;
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
	float3					position;
};

class BuildRHIMesh;

//
// BuildRenderThreadTaskUpdateModel
//
struct BuildRenderThreadTaskUpdateModel
{
	BaseModel *model;
	BuildRHIMesh *rhiMesh;
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

	bool useOrtho;
	bool forceHQShader;

	bool enableBlend;
	bool enableAlpha;

	float alphaBlendType;

	bool isFontImage;
	unsigned int texnum;
	void *renderMaterialHandle;

	bool drawpoly_srepeat;
	bool drawpoly_trepeat;

	float textureScale_X;
	float textureScale_Y;

	float4 spriteColor;

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
	forceHQShader = false;
	textureScale_X = 1;
	useOrtho = false;
	textureScale_Y = 1;
	renderMaterialHandle = NULL;
	spriteColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
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
	BuildRenderThreadTaskDrawLights			taskDrawLights;
	BuildRenderThreadDrawClassicScreen		taskDrawClassicScreen;
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
	void SetNormal(Build3DVector4 normal) { this->normal = normal; }

	Build3DVector4 position;
	Build3DVector4 uv;
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
		renderMaterialHandle = NULL;
		isDynamicPlane = false;
		buffer = NULL;
		indices = NULL;
		indicescount = 0;
		paletteNum = 0;
		shadeNum = 0;
		visibility = 0;

		ambient[0] = 0;
		ambient[1] = 0;
		ambient[2] = 0;

		fogColor[0] = 0.0f;
		fogColor[1] = 0.0f;
		fogColor[2] = 0.0f;

		fogDensity = 0;
		fogStart = 0;
		fogEnd = 0;

		isMaskWall = false;

		dynamic_vbo_offset = -1;
	}
	// geometry
	Build3DVertex*        buffer;
	int32_t				vertcount;

	int					paletteNum;
	int					shadeNum;
	int					visibility;

	float				fogColor[3];
	float				fogDensity;
	float				fogStart;
	float				fogEnd;

	// attributes
	float				tbn[3][3];
	float		        plane[4];

	byte				diffusemodulation[4];
	byte				ambient[4];

	int					tileNum;
	void				*renderMaterialHandle;

	int					vbo_offset;
	int					ibo_offset;

	bool				isMaskWall;

	bool				isDynamicPlane;

	// elements
	unsigned short*     indices;
	int32_t				indicescount;

	int					dynamic_vbo_offset;

	unsigned int		sectorNum;
	void				GetBoundsWorldSpace(float3 *mBoundingBoxCenterWorldSpace, float3 *mBoundingBoxHalfWorldSpace);
	void				GetBoundsObjectSpace(float3 *mBoundingBoxCenterObjectSpace, float3 *mBoundingBoxHalfObjectSpace)
	{
		float3 minPosition(FLT_MAX, FLT_MAX, FLT_MAX);
		float3 maxPosition(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		for (UINT ii = 0; ii < vertcount; ii++)
		{
			float3 position = float3(buffer[ii].position.x, buffer[ii].position.y, buffer[ii].position.z);
			minPosition = Min(minPosition, position);
			maxPosition = Max(maxPosition, position);
		}
		*mBoundingBoxCenterObjectSpace = (maxPosition + minPosition) * 0.5f;
		*mBoundingBoxHalfObjectSpace = (maxPosition - minPosition) * 0.5f;
	}
};

//
// Build3DPlane::UpdateBoundsWorldSpace
//
__forceinline void Build3DPlane::GetBoundsWorldSpace(float3 *mBoundingBoxCenterWorldSpace, float3 *mBoundingBoxHalfWorldSpace)
{
	// If an object is rigid, then it's object-space bounding box doesn't change.
	// However, if it moves, then it's world-space bounding box does change.
	// Call this function when the model moves
	// jmarshall
	float4x4 pWorld = (float4x4)float4x4Identity();
	float scaleSize = 0;

	if (currentModelViewMatrixCulling != NULL)
	{
		//pWorld = float4x4(currentModelViewMatrixCulling);
		scaleSize = 1;
	}
	float3 mBoundingBoxCenterObjectSpace, mBoundingBoxHalfObjectSpace;
	GetBoundsObjectSpace(&mBoundingBoxCenterObjectSpace, &mBoundingBoxHalfObjectSpace);

	float4 center = float4(mBoundingBoxCenterObjectSpace, 1.0f); // W = 1 because we want the xlation (i.e., center is a position)
	float4 half = float4(mBoundingBoxHalfObjectSpace, 0.0f); // W = 0 because we don't want xlation (i.e., half is a direction)

															 // TODO: optimize this
	float4 positions[8] = {
		center + float4(1.0f, 1.0f, 1.0f, 0.0f) * half,
		center + float4(1.0f, 1.0f,-1.0f, 0.0f) * half,
		center + float4(1.0f,-1.0f, 1.0f, 0.0f) * half,
		center + float4(1.0f,-1.0f,-1.0f, 0.0f) * half,
		center + float4(-1.0f, 1.0f, 1.0f, 0.0f) * half,
		center + float4(-1.0f, 1.0f,-1.0f, 0.0f) * half,
		center + float4(-1.0f,-1.0f, 1.0f, 0.0f) * half,
		center + float4(-1.0f,-1.0f,-1.0f, 0.0f) * half
	};
	// jmarshall end
	float4 minPosition(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
	float4 maxPosition(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);
	for (UINT ii = 0; ii < 8; ii++)
	{
		float4 position = (positions[ii] + scaleSize) * pWorld;
		minPosition = Min(minPosition, position);
		maxPosition = Max(maxPosition, position);
	}
	*mBoundingBoxCenterWorldSpace = ((maxPosition + minPosition) * 0.5f);
	*mBoundingBoxHalfWorldSpace = ((maxPosition - minPosition) * 0.5f);
}

//
// Build3DSector
//
struct Build3DSector 
{
	Build3DSector() {
		memset(this, 0, sizeof(Build3DSector));
		floor.vbo_offset = -1;
		floor.ibo_offset = -1;
		floor.dynamic_vbo_offset = -1;

		ceil.vbo_offset = -1;
		ceil.ibo_offset = -1;
		ceil.dynamic_vbo_offset = -1;

		boundingbox.Clear();

		ambientSectorId = 0;
	}

	const bool			IsCeilParalaxed() const {
		return ceilingstat & 1;
	}

	const bool			IsFloorParalaxed() const {
		return floorstat & 1;
	}

	int	ambientSectorId;

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
		int32_t     empty : 1;
		int32_t     uptodate : 1;
		int32_t     invalidtex : 1;
	}               flags;
	uint32_t        invalidid;

	bool			isInvalid;

	Math::AxisAlignedBox	boundingbox;
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
	Build3DSprite()
	{
		cacheModel = NULL;
	}
	Build3DPlane       plane;
	class CacheModel *cacheModel;

	float4x4			modelViewProjectionMatrix;
	float4x4		    modelMatrix;
	float4x4			modelViewInverse;
	float4x4		    ViewMatrix;
	uint32_t        hash;
	bool			isHorizsprite;
	bool			isWallSprite;
	int				paletteNum;
	bool			isVisible;
	byte			ambientColor[4];
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


class BuildFrameDrawPolyCmd
{
public:
	BuildFrameDrawPolyCmd();

	unsigned int globalpicnum;

	unsigned int numVertexes;
	Build3DVertex vertexpool[100];

	void SetTextureCoordinate(float x, float y);
	void SetCurrentVertex(int vertexId);
	void SetVertexColor(float r, float g, float b, float a);
	void SetPosition(float x, float y, float z);
private:
	int currentVertex;
};

BUILD3D_INLINE BuildFrameDrawPolyCmd::BuildFrameDrawPolyCmd()
{
	numVertexes = 0;
	globalpicnum = 0;
}

BUILD3D_INLINE void BuildFrameDrawPolyCmd::SetCurrentVertex(int vertexId)
{
	// Convert the triangle fan to strip.
//	if (vertexId % 2 == 0)
//		currentVertex = vertexId / 2;
//	else
//		currentVertex = numVertexes - 1 - vertexId / 2;
}

BUILD3D_INLINE void BuildFrameDrawPolyCmd::SetVertexColor(float r, float g, float b, float a)
{
//	vertexpool[currentVertex].binormal.x = r;
//	vertexpool[currentVertex].binormal.y = g;
//	vertexpool[currentVertex].binormal.z = b;
//	vertexpool[currentVertex].binormal.w = a;
}

BUILD3D_INLINE void BuildFrameDrawPolyCmd::SetTextureCoordinate(float x, float y)
{
//	vertexpool[currentVertex].uv.x = x;
//	vertexpool[currentVertex].uv.y = y;
}

BUILD3D_INLINE void BuildFrameDrawPolyCmd::SetPosition(float x, float y, float z)
{
//	vertexpool[currentVertex].position.x = x;
//	vertexpool[currentVertex].position.y = y;
//	vertexpool[currentVertex].position.z = z;
}


//
// Build3DPolymost
//
class Build3DBoardPolymost
{
public:
	void drawrooms();

	int GetNumDrawPolyCommands() {
		return numDrawPolyCommands;
	}

	BuildFrameDrawPolyCmd *GetRenderPolyCommands()
	{
		return &drawPolyCommands[0];
	}

	bool visibleSectorList[MAXSECTORS];
private:
	BuildFrameDrawPolyCmd *GetDrawPolyCommand() { return &drawPolyCommands[numDrawPolyCommands++]; }

	void domost(float x0, float y0, float x1, float y1, short sectorNum);
	void AddUniqueSector(short sectnum);
	void drawpoly(vec2f_t const * const dpxy, int32_t const n, int32_t method);
	void internal_nonparallaxed(vec2f_t n0, vec2f_t n1, float ryp0, float ryp1, float x0, float x1, float y0, float y1, int32_t sectnum);
	void calc_ypanning(int32_t refposz, float ryp0, float ryp1, float x0, float x1, uint8_t ypan, uint8_t yrepeat, int32_t dopancor);
	int32_t testvisiblemost(float const x0, float const x1);
	int getclosestpointonwall(vec2_t const * const pos, int32_t dawall, vec2_t * const n);
	void drawalls(int32_t const bunch);
	void scansector(int32_t sectnum);
	void editorfunc(void);
	int32_t bunchfront(const int32_t b1, const int32_t b2);
	void drawmaskwall(int32_t damaskwallcnt);
	int32_t lintersect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t x4, int32_t y4);
	int32_t findwall(tspritetype const * const tspr, int32_t * rd);
private:
	int numDrawPolyCommands;
	BuildFrameDrawPolyCmd drawPolyCommands[MAXWALLS];

	
};

void computeplane(Build3DPlane* p);

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

	Build3DBoardPolymost *GetGenericPolymostRenderer() {
		return &Polymost;
	}

	Build3DPlane **GetGlobalPlaneList() { return &planelist[0]; }
	int GetNumGlobalPlanes() { return planelist.size(); }
private:
	bool     buildfloor(int16_t sectnum);
	static void	 tesserror(int error);
	static void	 tessedgeflag(int error);
	static void  tessvertex(void* vertex, void* sector);
	float calc_ypancoef(char curypanning, int16_t curpicnum, int32_t dopancor);

	Build3DSector       *prsectors[MAXSECTORS];
	Build3DWall	        *prwalls[MAXWALLS];

	class BaseModel		*model;

	Build3DBoardPolymost Polymost;

	std::vector<Build3DPlane *> planelist;

	struct GLUtesselator*  prtess;
};

//
// Build3D
//
class Build3D
{
public:
	static void dorotatesprite(BuildRenderCommand &command, int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum, int8_t dashade, char dapalnum, int32_t dastat, uint8_t daalpha, int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2, int32_t uniqid);

	static void CalculateFogForPlane(int32_t tile, int32_t shade, int32_t vis, int32_t pal, Build3DPlane *plane);

	static int32_t printext256(int32_t xpos, int32_t ypos, int16_t col, int16_t backcol, const char *name, char fontsize);
private:
	static void drawpoly(BuildRenderThreadTaskRotateSprite	&taskRotateSprite, vec2f_t const * const dpxy, int32_t const n, int32_t method);
};

extern Build3D	build3D;

#undef small