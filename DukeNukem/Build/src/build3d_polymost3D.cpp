// build3d_polymost.cpp
//

#include "compat.h"
#include "build3d.h"
#include "build.h"
//#include "glbuild.h"
#include "mdsprite.h"
#include "pragmas.h"
#include "baselayer.h"
#include "osd.h"
#include "engine_priv.h"
#include "hightile.h"
#include "polymost.h"
#include "polymer.h"
#include "cache1d.h"
#include "kplib.h"
#include "texcache.h"
#include "common.h"

#include "Tesselation/GLU.h"
#include "PolymerNG/Models/Models.h"

/*
====================================================

This is a port of the original Polymost logic to a generic implementation. Polymost doesn't use index or vertex buffers which means it doesn't have to generate a bunch of planes. It basically does
tesselation on the fly. 

====================================================
*/


#ifndef _WIN32
extern int32_t filelength(int h); // kplib.c
#endif

extern char textfont[2048], smalltextfont[2048];

extern int32_t rendmode;

extern int32_t usemodels;
extern int32_t usehightile;
extern int32_t vsync;

#include <math.h> //<-important!
#include <float.h>

typedef struct { float x, cy[2], fy[2]; int32_t tag; int16_t n, p, ctag, ftag; } vsptyp;
#define VSPMAX 4096 //<- careful!
static vsptyp vsp[VSPMAX];
static int32_t gtag;

static float dxb1[MAXWALLSB], dxb2[MAXWALLSB];

#define SCISDIST 1.0f  //1.0: Close plane clipping distance

extern float shadescale;
extern int32_t shadescale_unbounded;

extern int32_t r_usenewshading;
extern int32_t r_usetileshades;
extern int32_t r_npotwallmode;

extern bool pow2xsplit;

static float gviewxrange;
static float ghoriz;
extern float gxyaspect;
extern float gyxscale, ghalfx, grhalfxdown10, grhalfxdown10x;
extern float gcosang, gsinang, gcosang2, gsinang2;
extern float gchang, gshang, gctang, gstang, gvisibility;
extern float gtang;

static vec3d_t xtex, ytex, otex;

extern float fcosglobalang, fsinglobalang;
extern float fxdim, fydim, fydimen, fviewingrange;
static int32_t preview_mouseaim = 1;  // when 1, displays a CROSSHAIR tsprite at the _real_ aimed position

static int32_t drawpoly_srepeat = 0, drawpoly_trepeat = 0;


extern int32_t glanisotropy;            // 0 = maximum supported by card



extern int32_t glusetexcompr;
extern int32_t glusetexcache, glusememcache;
extern int32_t r_polygonmode;     // 0:GL_FILL,1:GL_LINE,2:GL_POINT //FUK
extern int32_t glmultisample, glnvmultisamplehint;
static int32_t lastglpolygonmode; //FUK
extern int32_t r_detailmapping;
extern int32_t r_glowmapping;


extern int32_t gltexmaxsize ;      // 0 means autodetection on first run
extern int32_t gltexmiplevel;		// discards this many mipmap levels
extern int32_t glprojectionhacks;
static GLuint polymosttext;
extern int32_t glrendmode;

// This variable, and 'shadeforfullbrightpass' control the drawing of
// fullbright tiles.  Also see 'fullbrightloadingpass'.

extern int32_t r_fullbrights;
extern int32_t r_vertexarrays;
extern int32_t r_vbos;
extern int32_t r_vbocount;
extern int32_t r_animsmoothing;
extern int32_t r_downsize;
extern int32_t r_downsizevar;

// used for fogcalc
static float fogresult, fogresult2;
//coltypef fogcol, fogtable[MAXPALOOKUPS];

static const float float_trans[4] = { 1.0f, 1.0f, 0.66f, 0.33f };

static char ptempbuf[MAXWALLSB << 1];

// polymost ART sky control
extern int32_t r_parallaxskyclamping;
extern int32_t r_parallaxskypanning;

#define MIN_CACHETIME_PRINT 10

// this was faster in MSVC but slower with GCC... currently unknown on ARM where both
// the FPU and possibly the optimization path in the compiler need improvement
#if 0
static inline int32_t __float_as_int(float f) { return *(int32_t *)&f; }
static inline float __int_as_float(int32_t d) { return *(float *)&d; }
static inline float Bfabsf(float f) { return __int_as_float(__float_as_int(f) & 0x7fffffff); }
#else
#define Bfabsf fabsf
#endif

extern int32_t mdtims, omdtims;
extern uint8_t alphahackarray[MAXTILES];
extern int32_t drawingskybox;
extern int32_t hicprecaching;

extern hitdata_t polymost_hitdata;

// hack because we are not using this system.
hicreplctyp * hicfindsubst(int picnum, int palnum) { return NULL; }
hicreplctyp * hicfindskybox(int picnum, int palnum) { return NULL; }

void gltexinvalidate(int32_t dapicnum, int32_t dapalnum, int32_t dameth)
{
	
}

//Make all textures "dirty" so they reload, but not re-allocate
//This should be much faster than polymost_glreset()
//Use this for palette effects ... but not ones that change every frame!
void gltexinvalidatetype(int32_t type)
{
	
}

//void gltexapplyprops(void)
//{
//	if (getrendermode() == REND_CLASSIC)
//		return;
//
//	if (glinfo.maxanisotropy > 1.f)
//	{
//		if (glanisotropy <= 0 || glanisotropy > glinfo.maxanisotropy)
//			glanisotropy = (int32_t)glinfo.maxanisotropy;
//	}
//
//	gltexfiltermode = clamp(gltexfiltermode, 0, NUMGLFILTERMODES - 1);
//
//	for (int i = 0; i <= GLTEXCACHEADSIZ - 1; i++)
//	{
//		for (pthtyp *pth = texcache.list[i]; pth; pth = pth->next)
//		{
//			int32_t const filter = pth->flags & PTH_FORCEFILTER ? TEXFILTER_ON : -1;
//
//			bind_2d_texture(pth->glpic, filter);
//
//			if (r_fullbrights && pth->flags & PTH_HASFULLBRIGHT)
//				bind_2d_texture(pth->ofb->glpic, filter);
//		}
//	}
//
//	for (int i = 0; i<nextmodelid; i++)
//	{
//		md2model_t *m = (md2model_t *)models[i];
//
//		if (m->mdnum < 2)
//			continue;
//
//		for (int j = 0; j < m->numskins * (HICEFFECTMASK + 1); j++)
//		{
//			if (!m->texid[j])
//				continue;
//			bind_2d_texture(m->texid[j], -1);
//		}
//
//		for (mdskinmap_t *sk = m->skinmap; sk; sk = sk->next)
//			for (int j = 0; j < (HICEFFECTMASK + 1); j++)
//			{
//				if (!sk->texid[j])
//					continue;
//				bind_2d_texture(sk->texid[j], sk->flags & HICR_FORCEFILTER ? TEXFILTER_ON : -1);
//			}
//	}
//}

//--------------------------------------------------------------------------------------------------

static inline float polymost_invsqrt_approximation(float x)
{
#ifdef B_LITTLE_ENDIAN
	float const haf = x * .5f;
	struct conv { union { uint32_t i; float f; }; } *const n = (struct conv *)&x;
	n->i = 0x5f3759df - (n->i >> 1);
	return n->f * (1.5f - haf * (n->f * n->f));
#else
	// this is the comment
	return 1.f / Bsqrtf(x);
#endif
}

float glox1, gloy1, glox2, gloy2;

//Use this for both initialization and uninitialization of OpenGL.
static int32_t gltexcacnum = -1;
#define DAMETH_WALL 0

// one-time initialization of OpenGL for polymost
void polymost_glinit()
{

}

////////// VISIBILITY FOG ROUTINES //////////
//extern int32_t nofog;  // in windows/SDL layers

					   // only for r_usenewshading < 2 (not preferred)
static void fogcalc_old(int32_t shade, int32_t vis)
{
	float f;

	if (r_usenewshading == 1)
	{
		f = 0.9f * shade;
		f = (vis > 239) ? (float)(gvisibility*((vis - 240 + f))) :
			(float)(gvisibility*(vis + 16 + f));
	}
	else
	{
		f = (shade < 0) ? shade * 3.5f : shade * .66f;
		f = (vis > 239) ? (float)(gvisibility*((vis - 240 + f) / (klabs(vis - 256)))) :
			(float)(gvisibility*(vis + 16 + f));
	}

	if (f < 0.001f)
		f = 0.001f;
	else if (f > 100.0f)
		f = 100.0f;

	fogresult = f;
}

// For GL_LINEAR fog:
#define FOGDISTCONST 600
#define FULLVIS_BEGIN 2.9e30
#define FULLVIS_END 3.0e30

static inline void fogcalc(int32_t tile, int32_t shade, int32_t vis, int32_t pal)
{
	if (shade > 0 && getrendermode() == REND_POLYMOST && r_usetileshades == 1 &&
		!(globalflags & GLOBAL_NO_GL_TILESHADES) &&
		(!usehightile || !hicfindsubst(tile, pal)) &&
		(!usemodels || md_tilehasmodel(tile, pal) < 0))
		shade >>= 1;

//	fogcol = fogtable[pal];

	if (r_usenewshading < 2)
	{
		fogcalc_old(shade, vis);
		return;
	}

	float combvis = (float)globalvisibility * (uint8_t)(vis + 16);

	if (combvis == 0)
	{
		if (r_usenewshading == 2 && shade > 0)
		{
			// beg = -D*shade, end = D*(NUMSHADES-1-shade)
			//  => end/beg = -(NUMSHADES-1-shade)/shade
			fogresult = (float)-FULLVIS_BEGIN;
			fogresult2 = FULLVIS_BEGIN * (float)(numshades - 1 - shade) / shade;
		}
		else
		{
			fogresult = (float)FULLVIS_BEGIN;
			fogresult2 = (float)FULLVIS_END;
		}
	}
	else if (r_usenewshading == 3 && shade >= numshades - 1)
	{
		fogresult = -1;
		fogresult2 = 0;
	}
	else
	{
		combvis = 1.f / combvis;
		fogresult = (r_usenewshading == 3 && shade > 0) ? 0 : -(FOGDISTCONST * shade) * combvis;
		fogresult2 = (FOGDISTCONST * (numshades - 1 - shade)) * combvis;
	}
}

void calc_and_apply_fog(int32_t tile, int32_t shade, int32_t vis, int32_t pal)
{
	fogcalc(tile, shade, vis, pal);
	//bglFogfv(GL_FOG_COLOR, (GLfloat *)&fogcol);
	//
	//if (r_usenewshading < 2)
	//	bglFogf(GL_FOG_DENSITY, fogresult);
	//else
	//{
	//	bglFogf(GL_FOG_START, fogresult);
	//	bglFogf(GL_FOG_END, fogresult2);
	//}
}

void calc_and_apply_fog_factor(int32_t tile, int32_t shade, int32_t vis, int32_t pal, float factor)
{
	// NOTE: for r_usenewshading >= 2, the fog beginning/ending distance results are
	// unused.
	fogcalc(tile, shade, vis, pal);
	//bglFogfv(GL_FOG_COLOR, (GLfloat *)&fogcol);
	//
	//if (r_usenewshading < 2)
	//	bglFogf(GL_FOG_DENSITY, fogresult*factor);
	//else
	//{
	//	bglFogf(GL_FOG_START, (GLfloat)FULLVIS_BEGIN);
	//	bglFogf(GL_FOG_END, (GLfloat)FULLVIS_END);
	//}
}
////////////////////


static float get_projhack_ratio(void)
{
	if (glprojectionhacks)
	{
		float const mul = (gshang * gshang);
		return 1.05f + mul * mul * mul * mul;
	}

	// No projection hacks (legacy or new-aspect)
	return 1.f;
}

static void resizeglcheck(void)
{
//#ifndef EDUKE32_GLES
//	//FUK
//	if (lastglpolygonmode != r_polygonmode)
//	{
//		lastglpolygonmode = r_polygonmode;
//		switch (r_polygonmode)
//		{
//		default:
//		case 0:
//			bglPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
//		case 1:
//			bglPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
//		case 2:
//			bglPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
//		}
//	}
//	if (r_polygonmode) //FUK
//	{
//		bglClearColor(1.0, 1.0, 1.0, 0.0);
//		bglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		bglDisable(GL_TEXTURE_2D);
//	}
//#else
//	bglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//#endif

//	if ((glox1 != windowx1) || (gloy1 != windowy1) || (glox2 != windowx2) || (gloy2 != windowy2))
//	{
//		const int32_t ourxdimen = (windowx2 - windowx1 + 1);
//		float ratio = get_projhack_ratio();
//		const int32_t fovcorrect = (int32_t)(ourxdimen*ratio - ourxdimen);
//
//		ratio = 1.f / ratio;
//
//		glox1 = (float)windowx1; gloy1 = (float)windowy1;
//		glox2 = (float)windowx2; gloy2 = (float)windowy2;
//
//		bglViewport(windowx1 - (fovcorrect / 2), yres - (windowy2 + 1),
//			ourxdimen + fovcorrect, windowy2 - windowy1 + 1);
//
//		bglMatrixMode(GL_PROJECTION);
//
//		float m[4][4];
//		Bmemset(m, 0, sizeof(m));
//
//
//		m[0][0] = fydimen * ratio; m[0][2] = 1.f;
//		m[1][1] = fxdimen; m[1][2] = 1.f;
//		m[2][2] = 1.f; m[2][3] = fydimen * ratio;
//		m[3][2] = -1.f;
//		bglLoadMatrixf(&m[0][0]);
//
//		bglMatrixMode(GL_MODELVIEW);
//		bglLoadIdentity();
//
//		if (!nofog) bglEnable(GL_FOG);
//
//		//bglEnable(GL_TEXTURE_2D);
//	}
}
/*
static void fixtransparency(coltype *dapic, vec2_t dasiz, vec2_t dasiz2, int32_t dameth)
{
	vec2_t doxy = { dasiz2.x - 1, dasiz2.y - 1 };

	if (dameth & 4) { doxy.x = min(doxy.x, dasiz.x); doxy.y = min(doxy.y, dasiz.y); }
	else { dasiz = dasiz2; } //Make repeating textures duplicate top/left parts

	dasiz.x--; dasiz.y--; //Hacks for optimization inside loop
	int32_t const naxsiz2 = -dasiz2.x;

	//Set transparent pixels to average color of neighboring opaque pixels
	//Doing this makes bilinear filtering look much better for masked textures (I.E. sprites)
	for (int y = doxy.y; y >= 0; y--)
	{
		coltype * wpptr = &dapic[y*dasiz2.x + doxy.x];

		for (int x = doxy.x; x >= 0; x--, wpptr--)
		{
			if (wpptr->a) continue;

			int r = 0, g = 0, b = 0, j = 0;

			if ((x>     0) && (wpptr[-1].a)) { r += wpptr[-1].r; g += wpptr[-1].g; b += wpptr[-1].b; j++; }
			if ((x<dasiz.x) && (wpptr[+1].a)) { r += wpptr[+1].r; g += wpptr[+1].g; b += wpptr[+1].b; j++; }
			if ((y>     0) && (wpptr[naxsiz2].a)) { r += wpptr[naxsiz2].r; g += wpptr[naxsiz2].g; b += wpptr[naxsiz2].b; j++; }
			if ((y<dasiz.y) && (wpptr[dasiz2.x].a)) { r += wpptr[dasiz2.x].r; g += wpptr[dasiz2.x].g; b += wpptr[dasiz2.x].b; j++; }

			switch (j)
			{
			case 1:
				wpptr->r = r; wpptr->g = g; wpptr->b = b; break;
			case 2:
				wpptr->r = ((r + 1) >> 1); wpptr->g = ((g + 1) >> 1); wpptr->b = ((b + 1) >> 1); break;
			case 3:
				wpptr->r = ((r * 85 + 128) >> 8); wpptr->g = ((g * 85 + 128) >> 8); wpptr->b = ((b * 85 + 128) >> 8); break;
			case 4:
				wpptr->r = ((r + 2) >> 2); wpptr->g = ((g + 2) >> 2); wpptr->b = ((b + 2) >> 2); break;
			}
		}
	}
}
*/

/*
void uploadtexture(int32_t doalloc, vec2_t siz, int32_t intexfmt, int32_t texfmt,
	coltype *pic, vec2_t tsiz, int32_t dameth)
{
	const int hi = !!(dameth & DAMETH_HI);
	const int nocompress = !!(dameth & DAMETH_NOCOMPRESS);
	const int nomiptransfix = !!(dameth & DAMETH_NOFIX);

	dameth &= ~(DAMETH_HI | DAMETH_NOCOMPRESS | DAMETH_NOFIX);

	if (gltexmaxsize <= 0)
	{
		GLint i = 0;
		bglGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
		if (!i) gltexmaxsize = 6;   // 2^6 = 64 == default GL max texture size
		else
		{
			gltexmaxsize = 0;
			for (; i>1; i >>= 1) gltexmaxsize++;
		}
	}

	gltexmiplevel = max(0, min(gltexmaxsize - 1, gltexmiplevel));

	int miplevel = gltexmiplevel;

	while ((siz.x >> miplevel) > (1 << gltexmaxsize) || (siz.y >> miplevel) > (1 << gltexmaxsize))
		miplevel++;

	if (hi && !nocompress && r_downsize > miplevel)
		miplevel = r_downsize;

	if (!miplevel)
	{
		if (doalloc & 1)
			bglTexImage2D(GL_TEXTURE_2D, 0, intexfmt, siz.x, siz.y, 0, texfmt, GL_UNSIGNED_BYTE, pic); //loading 1st time
		else
			bglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, siz.x, siz.y, texfmt, GL_UNSIGNED_BYTE, pic); //overwrite old texture
	}

#if 0
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, xsiz, ysiz, texfmt, GL_UNSIGNED_BYTE, pic); //Needs C++ to link?
#elif 1
	vec2_t siz2 = siz;

	for (int j = 1; (siz2.x > 1) || (siz2.y > 1); j++)
	{
		vec2_t const siz3 = { max(1, siz2.x >> 1), max(1, siz2.y >> 1) };  // this came from the GL_ARB_texture_non_power_of_two spec
																		   //x3 = ((x2+1)>>1); y3 = ((y2+1)>>1);

		for (int y = 0; y<siz3.y; y++)
		{
			coltype *wpptr = &pic[y*siz3.x];
			coltype const *rpptr = &pic[(y << 1)*siz2.x];

			for (int x = 0; x<siz3.x; x++, wpptr++, rpptr += 2)
			{
				int32_t r = 0, g = 0, b = 0, a = 0, k = 0;

				if (rpptr[0].a) { r += rpptr[0].r; g += rpptr[0].g; b += rpptr[0].b; a += rpptr[0].a; k++; }
				if ((x + x + 1 < siz2.x) && (rpptr[1].a)) { r += rpptr[1].r; g += rpptr[1].g; b += rpptr[1].b; a += rpptr[1].a; k++; }
				if (y + y + 1 < siz2.y)
				{
					if ((rpptr[siz2.x].a)) { r += rpptr[siz2.x].r; g += rpptr[siz2.x].g; b += rpptr[siz2.x].b; a += rpptr[siz2.x].a; k++; }
					if ((x + x + 1 < siz2.x) && (rpptr[siz2.x + 1].a)) { r += rpptr[siz2.x + 1].r; g += rpptr[siz2.x + 1].g; b += rpptr[siz2.x + 1].b; a += rpptr[siz2.x + 1].a; k++; }
				}
				switch (k)
				{
				case 0:
				case 1:
					wpptr->r = r; wpptr->g = g; wpptr->b = b; wpptr->a = a; break;
				case 2:
					wpptr->r = ((r + 1) >> 1); wpptr->g = ((g + 1) >> 1); wpptr->b = ((b + 1) >> 1); wpptr->a = ((a + 1) >> 1); break;
				case 3:
					wpptr->r = ((r * 85 + 128) >> 8); wpptr->g = ((g * 85 + 128) >> 8); wpptr->b = ((b * 85 + 128) >> 8); wpptr->a = ((a * 85 + 128) >> 8); break;
				case 4:
					wpptr->r = ((r + 2) >> 2); wpptr->g = ((g + 2) >> 2); wpptr->b = ((b + 2) >> 2); wpptr->a = ((a + 2) >> 2); break;
				default:
					EDUKE32_UNREACHABLE_SECTION(break);
				}
				//if (wpptr->a) wpptr->a = 255;
			}
		}

		if (!nomiptransfix)
		{
			vec2_t const tsizzle = { (tsiz.x + (1 << j) - 1) >> j, (tsiz.y + (1 << j) - 1) >> j };
			vec2_t const mnizzle = { siz3.x, siz3.y };

			fixtransparency(pic, tsizzle, mnizzle, dameth);
		}

		if (j >= miplevel)
		{
			if (doalloc & 1) // loading 1st time
				bglTexImage2D(GL_TEXTURE_2D, j - miplevel, intexfmt, siz3.x, siz3.y, 0, texfmt, GL_UNSIGNED_BYTE, pic);
			else             // overwrite old texture
				bglTexSubImage2D(GL_TEXTURE_2D, j - miplevel, 0, 0, siz3.x, siz3.y, texfmt, GL_UNSIGNED_BYTE, pic);
		}

		siz2 = siz3;
	}
#endif
}
*/

#if 0
// TODO: make configurable
static int32_t tile_is_sky(int32_t tilenum)
{
	return return (tilenum >= 78 /*CLOUDYOCEAN*/ && tilenum <= 99 /*REDSKY2*/);
}
#else
# define tile_is_sky(x) (0)
#endif

/*
static void polymost_setuptexture(const int32_t dameth, int filter)
{
	const GLuint clamp_mode = glinfo.clamptoedge ? GL_CLAMP_TO_EDGE : GL_CLAMP;

	gltexfiltermode = clamp(gltexfiltermode, 0, NUMGLFILTERMODES - 1);

	if (filter == -1)
		filter = gltexfiltermode;

	bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glfiltermodes[filter].mag);
	bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glfiltermodes[filter].min);

#ifndef EDUKE32_GLES
	if (glinfo.maxanisotropy > 1.f)
	{
		uint32_t i = (unsigned)Blrintf(glinfo.maxanisotropy);

		if ((unsigned)glanisotropy > i)
			glanisotropy = i;

		bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, glanisotropy);
	}
#endif

	if (!(dameth & DAMETH_CLAMPED))
	{
		bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, !tile_is_sky(dapic) ? GL_REPEAT : clamp_mode);
		bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		// For sprite textures, clamping looks better than wrapping
		bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp_mode);
		bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp_mode);
	}
}

void gloadtile_art(int32_t dapic, int32_t dapal, int32_t tintpalnum, int32_t dashade, int32_t dameth, pthtyp *pth, int32_t doalloc)
{
	static int32_t fullbrightloadingpass = 0;

	vec2_t siz, tsiz = tilesiz[dapic];

	if (!glinfo.texnpot)
	{
		for (siz.x = 1; siz.x < tsiz.x; siz.x += siz.x);
		for (siz.y = 1; siz.y < tsiz.y; siz.y += siz.y);
	}
	else
	{
		if ((tsiz.x | tsiz.y) == 0)
			siz.x = siz.y = 1;
		else
			siz = tsiz;
	}

	coltype *pic = (coltype *)Xmalloc(siz.x*siz.y*sizeof(coltype));
	char hasalpha = 0, hasfullbright = 0;

	if (!waloff[dapic])
	{
		//Force invalid textures to draw something - an almost purely transparency texture
		//This allows the Z-buffer to be updated for mirrors (which are invalidated textures)
		pic[0].r = pic[0].g = pic[0].b = 0; pic[0].a = 1;
		tsiz.x = tsiz.y = 1; hasalpha = 1;
	}
	else
	{
		const int dofullbright = !(picanm[dapic].sf & PICANM_NOFULLBRIGHT_BIT) && !(globalflags & GLOBAL_NO_GL_FULLBRIGHT);

		for (int y = 0; y < siz.y; y++)
		{
			coltype *wpptr = &pic[y * siz.x];
			int32_t y2 = (y < tsiz.y) ? y : y - tsiz.y;

			for (int x = 0; x < siz.x; x++, wpptr++)
			{
				int32_t dacol;
				int32_t x2 = (x < tsiz.x) ? x : x - tsiz.x;

				if ((dameth & DAMETH_CLAMPED) && (x >= tsiz.x || y >= tsiz.y)) //Clamp texture
				{
					wpptr->r = wpptr->g = wpptr->b = wpptr->a = 0;
					continue;
				}

				dacol = *(char *)(waloff[dapic] + x2*tsiz.y + y2);

				if (!fullbrightloadingpass)
				{
					// regular texture
					if (IsPaletteIndexFullbright(dacol) && dofullbright)
						hasfullbright = 1;

					wpptr->a = 255;
				}
				else
				{
					// texture with only fullbright areas
					if (!IsPaletteIndexFullbright(dacol))    // regular colors
					{
						wpptr->a = 0;
						hasalpha = 1;
					}
					else   // fullbright
					{
						wpptr->a = 255;
					}
				}

				if (dacol != 255)
				{
					char *p = (char *)(palookup[dapal]) + (int32_t)(dashade << 8);
					dacol = (uint8_t)p[dacol];
				}
				else
				{
					wpptr->a = 0;
					hasalpha = 1;
				}

				bricolor((palette_t *)wpptr, dacol);

				if (!fullbrightloadingpass && tintpalnum >= 0)
				{
					uint8_t const r = hictinting[tintpalnum].r;
					uint8_t const g = hictinting[tintpalnum].g;
					uint8_t const b = hictinting[tintpalnum].b;
					uint8_t const effect = hictinting[tintpalnum].f;

					if (effect & HICTINT_GRAYSCALE)
					{
						wpptr->g = wpptr->r = wpptr->b = (uint8_t)((wpptr->r * GRAYSCALE_COEFF_RED) +
							(wpptr->g * GRAYSCALE_COEFF_GREEN) +
							(wpptr->b * GRAYSCALE_COEFF_BLUE));
					}

					if (effect & HICTINT_INVERT)
					{
						wpptr->b = 255 - wpptr->b;
						wpptr->g = 255 - wpptr->g;
						wpptr->r = 255 - wpptr->r;
					}

					if (effect & HICTINT_COLORIZE)
					{
						wpptr->b = min((int32_t)((wpptr->b) * b) >> 6, 255);
						wpptr->g = min((int32_t)((wpptr->g) * g) >> 6, 255);
						wpptr->r = min((int32_t)((wpptr->r) * r) >> 6, 255);
					}

					switch (effect & HICTINT_BLENDMASK)
					{
					case HICTINT_BLEND_SCREEN:
						wpptr->b = 255 - (((255 - wpptr->b) * (255 - b)) >> 8);
						wpptr->g = 255 - (((255 - wpptr->g) * (255 - g)) >> 8);
						wpptr->r = 255 - (((255 - wpptr->r) * (255 - r)) >> 8);
						break;
					case HICTINT_BLEND_OVERLAY:
						wpptr->b = wpptr->b < 128 ? (wpptr->b * b) >> 7 : 255 - (((255 - wpptr->b) * (255 - b)) >> 7);
						wpptr->g = wpptr->g < 128 ? (wpptr->g * g) >> 7 : 255 - (((255 - wpptr->g) * (255 - g)) >> 7);
						wpptr->r = wpptr->r < 128 ? (wpptr->r * r) >> 7 : 255 - (((255 - wpptr->r) * (255 - r)) >> 7);
						break;
					case HICTINT_BLEND_HARDLIGHT:
						wpptr->b = b < 128 ? (wpptr->b * b) >> 7 : 255 - (((255 - wpptr->b) * (255 - b)) >> 7);
						wpptr->g = g < 128 ? (wpptr->g * g) >> 7 : 255 - (((255 - wpptr->g) * (255 - g)) >> 7);
						wpptr->r = r < 128 ? (wpptr->r * r) >> 7 : 255 - (((255 - wpptr->r) * (255 - r)) >> 7);
						break;
					}
				}
			}
		}
	}

	if (doalloc) bglGenTextures(1, (GLuint *)&pth->glpic); //# of textures (make OpenGL allocate structure)
	bglBindTexture(GL_TEXTURE_2D, pth->glpic);

	fixtransparency(pic, tsiz, siz, dameth);

	int32_t npoty = 0;

	if (polymost_want_npotytex(dameth, siz.y) && tsiz.x == siz.x && tsiz.y == siz.y)  // XXX
	{
		const int32_t nextpoty = 1 << ((picsiz[dapic] >> 4) + 1);
		const int32_t ydif = nextpoty - siz.y;
		coltype *paddedpic;

		Bassert(ydif < siz.y);

		paddedpic = (coltype *)Xrealloc(pic, siz.x * nextpoty * sizeof(coltype));

		pic = paddedpic;
		Bmemcpy(&pic[siz.x * siz.y], pic, siz.x * ydif * sizeof(coltype));
		siz.y = tsiz.y = nextpoty;

		npoty = PTH_NPOTWALL;
	}

	uploadtexture(doalloc, siz, hasalpha ? GL_RGBA : GL_RGB, GL_RGBA, pic, tsiz, dameth);

	Bfree(pic);

	polymost_setuptexture(dameth, -1);

	pth->picnum = dapic;
	pth->palnum = dapal;
	pth->shade = dashade;
	pth->effects = 0;
	pth->flags = TO_PTH_CLAMPED(dameth) | (hasalpha*PTH_HASALPHA) | npoty;
	pth->hicr = NULL;

	if (hasfullbright && !fullbrightloadingpass)
	{
		// Load the ONLY texture that'll be assembled with the regular one to
		// make the final texture with fullbright pixels.
		fullbrightloadingpass = 1;

		pth->ofb = (pthtyp *)Xcalloc(1, sizeof(pthtyp));
		pth->flags |= PTH_HASFULLBRIGHT;

		gloadtile_art(dapic, dapal, -1, 0, dameth, pth->ofb, 1);

		fullbrightloadingpass = 0;
	}
}

int32_t gloadtile_hi(int32_t dapic, int32_t dapalnum, int32_t facen, hicreplctyp *hicr,
	int32_t dameth, pthtyp *pth, int32_t doalloc, char effect)
{
	if (!hicr) return -1;

	coltype *pic = NULL;

	char *fn;
	int32_t picfillen, intexfmt = GL_RGBA, filh;

	int32_t startticks = 0, willprint = 0;

	if (facen > 0)
	{
		if (!hicr->skybox || facen > 6 || !hicr->skybox->face[facen - 1])
			return -1;

		fn = hicr->skybox->face[facen - 1];
	}
	else
	{
		if (!hicr->filename)
			return -1;

		fn = hicr->filename;
	}

	if (EDUKE32_PREDICT_FALSE((filh = kopen4load(fn, 0)) < 0))
	{
		OSD_Printf("hightile: %s (pic %d) not found\n", fn, dapic);
		return -2;
	}

	picfillen = kfilelength(filh);

	kclose(filh);	// FIXME: shouldn't have to do this. bug in cache1d.c

	char hasalpha = 255;
	texcacheheader cachead;
	int32_t gotcache = texcache_readtexheader(fn, picfillen + (dapalnum << 8), dameth, effect, &cachead, 0);
	vec2_t siz = { 0, 0 }, tsiz = { 0, 0 };

	if (gotcache && !texcache_loadtile(&cachead, &doalloc, pth))
	{
		tsiz.x = cachead.xdim;
		tsiz.y = cachead.ydim;
		hasalpha = (cachead.flags & CACHEAD_HASALPHA) ? 0 : 255;
	}
	else
	{
		int32_t r, g, b;
		int32_t j, y;
		int32_t isart = 0;

		gotcache = 0;	// the compressed version will be saved to disk

		int32_t const length = kpzbufload(fn);
		if (length == 0)
			return -1;

		// tsizx/y = replacement texture's natural size
		// xsiz/y = 2^x size of replacement

#ifdef WITHKPLIB
		kpgetdim(kpzbuf, picfillen, &tsiz.x, &tsiz.y);
#endif

		if (tsiz.x == 0 || tsiz.y == 0)
		{
			if (E_CheckUnitArtFileHeader((uint8_t *)kpzbuf, picfillen))
				return -1;

			tsiz.x = B_LITTLE16(B_UNBUF16(&kpzbuf[16]));
			tsiz.y = B_LITTLE16(B_UNBUF16(&kpzbuf[18]));

			if (tsiz.x == 0 || tsiz.y == 0)
				return -1;

			isart = 1;
		}

		pth->siz = tsiz;

		if (!glinfo.texnpot)
		{
			for (siz.x = 1; siz.x<tsiz.x; siz.x += siz.x);
			for (siz.y = 1; siz.y<tsiz.y; siz.y += siz.y);
		}
		else
			siz = tsiz;

		if (isart)
		{
			if (tsiz.x * tsiz.y + ARTv1_UNITOFFSET > picfillen)
				return -2;
		}

		int32_t const bytesperline = siz.x * sizeof(coltype);
		pic = (coltype *)Xcalloc(siz.y, bytesperline);

		startticks = getticks();

		static coltype *lastpic = NULL;
		static char *lastfn = NULL;
		static int32_t lastsize = 0;

		if (lastpic && lastfn && !Bstrcmp(lastfn, fn))
		{
			willprint = 1;
			Bmemcpy(pic, lastpic, siz.x*siz.y*sizeof(coltype));
		}
		else
		{
			if (isart)
			{
				E_RenderArtDataIntoBuffer((palette_t *)pic, (uint8_t *)&kpzbuf[ARTv1_UNITOFFSET], siz.x, tsiz.x, tsiz.y);
			}
#ifdef WITHKPLIB
			else
			{
				if (kprender(kpzbuf, picfillen, (intptr_t)pic, bytesperline, siz.x, siz.y))
				{
					Bfree(pic);
					return -2;
				}
			}
#endif

			willprint = 2;

			if (hicprecaching)
			{
				lastfn = fn;  // careful...
				if (!lastpic)
				{
					lastpic = (coltype *)Bmalloc(siz.x*siz.y*sizeof(coltype));
					lastsize = siz.x*siz.y;
				}
				else if (lastsize < siz.x*siz.y)
				{
					Bfree(lastpic);
					lastpic = (coltype *)Bmalloc(siz.x*siz.y*sizeof(coltype));
				}
				if (lastpic)
					Bmemcpy(lastpic, pic, siz.x*siz.y*sizeof(coltype));
			}
			else if (lastpic)
			{
				DO_FREE_AND_NULL(lastpic);
				lastfn = NULL;
				lastsize = 0;
			}
		}

		r = (glinfo.bgra) ? hictinting[dapalnum].r : hictinting[dapalnum].b;
		g = hictinting[dapalnum].g;
		b = (glinfo.bgra) ? hictinting[dapalnum].b : hictinting[dapalnum].r;

		for (y = 0, j = 0; y < tsiz.y; y++, j += siz.x)
		{
			coltype tcol;
			char *cptr = britable[gammabrightness ? 0 : curbrightness];
			coltype *rpptr = &pic[j];

			int32_t x;

			for (x = 0; x<tsiz.x; x++)
			{
				tcol.b = cptr[rpptr[x].b];
				tcol.g = cptr[rpptr[x].g];
				tcol.r = cptr[rpptr[x].r];
				tcol.a = rpptr[x].a;
				hasalpha &= rpptr[x].a;

				if (effect & HICTINT_GRAYSCALE)
				{
					tcol.g = tcol.r = tcol.b = (uint8_t)((tcol.b * GRAYSCALE_COEFF_RED) +
						(tcol.g * GRAYSCALE_COEFF_GREEN) +
						(tcol.r * GRAYSCALE_COEFF_BLUE));
				}

				if (effect & HICTINT_INVERT)
				{
					tcol.b = 255 - tcol.b;
					tcol.g = 255 - tcol.g;
					tcol.r = 255 - tcol.r;
				}

				if (effect & HICTINT_COLORIZE)
				{
					tcol.b = min((int32_t)((tcol.b) * r) >> 6, 255);
					tcol.g = min((int32_t)((tcol.g) * g) >> 6, 255);
					tcol.r = min((int32_t)((tcol.r) * b) >> 6, 255);
				}

				switch (effect & HICTINT_BLENDMASK)
				{
				case HICTINT_BLEND_SCREEN:
					tcol.b = 255 - (((255 - tcol.b) * (255 - r)) >> 8);
					tcol.g = 255 - (((255 - tcol.g) * (255 - g)) >> 8);
					tcol.r = 255 - (((255 - tcol.r) * (255 - b)) >> 8);
					break;
				case HICTINT_BLEND_OVERLAY:
					tcol.b = tcol.b < 128 ? (tcol.b * r) >> 7 : 255 - (((255 - tcol.b) * (255 - r)) >> 7);
					tcol.g = tcol.g < 128 ? (tcol.g * g) >> 7 : 255 - (((255 - tcol.g) * (255 - g)) >> 7);
					tcol.r = tcol.r < 128 ? (tcol.r * b) >> 7 : 255 - (((255 - tcol.r) * (255 - b)) >> 7);
					break;
				case HICTINT_BLEND_HARDLIGHT:
					tcol.b = r < 128 ? (tcol.b * r) >> 7 : 255 - (((255 - tcol.b) * (255 - r)) >> 7);
					tcol.g = g < 128 ? (tcol.g * g) >> 7 : 255 - (((255 - tcol.g) * (255 - g)) >> 7);
					tcol.r = b < 128 ? (tcol.r * b) >> 7 : 255 - (((255 - tcol.r) * (255 - b)) >> 7);
					break;
				}

				rpptr[x] = tcol;
			}
		}

		if ((!(dameth & DAMETH_CLAMPED)) || facen) //Duplicate texture pixels (wrapping tricks for non power of 2 texture sizes)
		{
			if (siz.x > tsiz.x)  // Copy left to right
			{
				int32_t *lptr = (int32_t *)pic;

				for (y = 0; y < tsiz.y; y++, lptr += siz.x)
					Bmemcpy(&lptr[tsiz.x], lptr, (siz.x - tsiz.x) << 2);
			}

			if (siz.y > tsiz.y)  // Copy top to bottom
				Bmemcpy(&pic[siz.x * tsiz.y], pic, (siz.y - tsiz.y) * siz.x << 2);
		}

		int32_t texfmt;

		if (!glinfo.bgra)
		{
			texfmt = GL_RGBA;

			for (j = siz.x*siz.y - 1; j >= 0; j--)
				swapchar(&pic[j].r, &pic[j].b);
		}
		else texfmt = GL_BGRA;

		if (tsiz.x >> r_downsize <= tilesiz[dapic].x || tsiz.y >> r_downsize <= tilesiz[dapic].y)
			hicr->flags |= (HICR_NOCOMPRESS + HICR_NOSAVE);

#if !defined EDUKE32_GLES
		if (glinfo.texcompr && glusetexcompr && !(hicr->flags & HICR_NOSAVE))
			intexfmt = (hasalpha == 255) ? GL_COMPRESSED_RGB_ARB : GL_COMPRESSED_RGBA_ARB;
		else
#endif
			if (hasalpha == 255) intexfmt = GL_RGB;

		if ((doalloc & 3) == 1)
			bglGenTextures(1, &pth->glpic); //# of textures (make OpenGL allocate structure)
		bglBindTexture(GL_TEXTURE_2D, pth->glpic);

		fixtransparency(pic, tsiz, siz, dameth);
		uploadtexture(doalloc, siz, intexfmt, texfmt, pic, tsiz,
			dameth | DAMETH_HI | DAMETH_NOFIX | (hicr->flags & HICR_NOCOMPRESS ? DAMETH_NOCOMPRESS : 0));
	}

	// precalculate scaling parameters for replacement
	if (facen > 0)
	{
		pth->scale.x = (float)tsiz.x * (1.0f / 64.f);
		pth->scale.y = (float)tsiz.y * (1.0f / 64.f);
	}
	else
	{
		pth->scale.x = (float)tsiz.x / (float)tilesiz[dapic].x;
		pth->scale.y = (float)tsiz.y / (float)tilesiz[dapic].y;
	}

	polymost_setuptexture(dameth, hicr->flags & HICR_FORCEFILTER ? TEXFILTER_ON : -1);

	DO_FREE_AND_NULL(pic);

	if (tsiz.x >> r_downsize <= tilesiz[dapic].x || tsiz.y >> r_downsize <= tilesiz[dapic].y)
		hicr->flags |= HICR_NOCOMPRESS | HICR_NOSAVE;

	pth->picnum = dapic;
	pth->effects = effect;
	pth->flags = TO_PTH_CLAMPED(dameth) | PTH_HIGHTILE | ((facen>0) * PTH_SKYBOX) | ((hasalpha != 255) ? PTH_HASALPHA : 0) |
		(hicr->flags & HICR_FORCEFILTER ? PTH_FORCEFILTER : 0);
	pth->skyface = facen;
	pth->hicr = hicr;

	if (!gotcache && glinfo.texcompr && glusetexcompr && glusetexcache && !(hicr->flags & HICR_NOSAVE))
	{
		const int32_t nonpow2 = check_nonpow2(siz.x) || check_nonpow2(siz.y);

		// save off the compressed version
		cachead.quality = (hicr->flags & HICR_NOCOMPRESS) ? 0 : r_downsize;
		cachead.xdim = tsiz.x >> cachead.quality;
		cachead.ydim = tsiz.y >> cachead.quality;

		// handle nocompress:
		cachead.flags = nonpow2 * CACHEAD_NONPOW2 | (hasalpha != 255 ? CACHEAD_HASALPHA : 0) |
			(hicr->flags & HICR_NOCOMPRESS ? CACHEAD_NOCOMPRESS : 0);

		///            OSD_Printf("Caching \"%s\"\n", fn);
		texcache_writetex(fn, picfillen + (dapalnum << 8), dameth, effect, &cachead);

		if (willprint)
		{
			int32_t etime = getticks() - startticks;
			if (etime >= MIN_CACHETIME_PRINT)
				OSD_Printf("Load tile %4d: p%d-m%d-e%d %s... cached... %d ms\n", dapic, dapalnum, dameth, effect,
					willprint == 2 ? fn : "", etime);
			willprint = 0;
		}
		else
			OSD_Printf("Cached \"%s\"\n", fn);
	}

	if (willprint)
	{
		int32_t etime = getticks() - startticks;
		if (etime >= MIN_CACHETIME_PRINT)
			OSD_Printf("Load tile %4d: p%d-m%d-e%d %s... %d ms\n", dapic, dapalnum, dameth, effect,
				willprint == 2 ? fn : "", etime);
	}

	return 0;
}
*/

static inline float getshadefactor(int32_t const shade)
{
	int32_t const shadebound = (shadescale_unbounded || shade >= numshades) ? numshades : numshades - 1;
	float const clamped_shade = min(max(shade*shadescale, 0), shadebound);

	return ((float)(numshades - clamped_shade)) / (float)numshades;
}

void Build3DBoardPolymost::drawpoly(vec2f_t const * const dpxy, int32_t const n, int32_t method)
{
#if 0
	BuildFrameDrawPolyCmd *drawPolyCmd = GetDrawPolyCommand();

#ifdef YAX_ENABLE
	if (g_nodraw) return;
#endif

	if (method == -1 || (uint32_t)globalpicnum >= MAXTILES) return;

	const int32_t method_ = method;

	if (n == 3)
	{
		if ((dpxy[0].x - dpxy[1].x) * (dpxy[2].y - dpxy[1].y) >=
			(dpxy[2].x - dpxy[1].x) * (dpxy[0].y - dpxy[1].y)) return; //for triangle
	}
	else
	{
		float f = 0; //f is area of polygon / 2

		for (int i = n - 2, j = n - 1, k = 0; k<n; i = j, j = k, k++)
			f += (dpxy[i].x - dpxy[k].x)*dpxy[j].y;

		if (f <= 0) return;
	}

	if (palookup[globalpal] == NULL)
		globalpal = 0;

	drawPolyCmd->globalpicnum = globalpicnum;

	//Load texture (globalpicnum)
	//setgotpic(globalpicnum);
	vec2_t tsiz = tilesiz[globalpicnum];

	if (!waloff[globalpicnum])
	{
		//loadtile(globalpicnum);

		if (!waloff[globalpicnum])
		{
			tsiz.x = tsiz.y = 1;
			method = 1; //Hack to update Z-buffer for invalid mirror textures
		}
	}

	Bassert(n <= 8);

	int j = 0;
	float px[8], py[8], dd[8], uu[8], vv[8];
	float const ozgs = ghalfx * gshang,
		ozgc = ghalfx * gchang;

	for (int i = 0; i<n; ++i)
	{
		//Up/down rotation
		vec3f_t const orot = { dpxy[i].x - ghalfx,
			(dpxy[i].y - ghoriz) * gchang - ozgs,
			(dpxy[i].y - ghoriz) * gshang + ozgc };

		// Tilt rotation
		float const r = ghalfx / orot.z;

		px[j] = ghalfx + (((orot.x * gctang) - (orot.y * gstang)) * r);
		py[j] = ghoriz + (((orot.x * gstang) + (orot.y * gctang)) * r);

		dd[j] = (dpxy[i].x * xtex.d + dpxy[i].y * ytex.d + otex.d) * r;
		uu[j] = (dpxy[i].x * xtex.u + dpxy[i].y * ytex.u + otex.u) * r;
		vv[j] = (dpxy[i].x * xtex.v + dpxy[i].y * ytex.v + otex.v) * r;

		if ((!j) || (px[j] != px[j - 1]) || (py[j] != py[j - 1]))
			j++;
	}

	while ((j >= 3) && (px[j - 1] == px[0]) && (py[j - 1] == py[0])) j--;

	if (j < 3)
		return;

	int const npoints = j;

	//if (skyclamphack) method |= DAMETH_CLAMPED;

	//pthtyp *pth = our_texcache_fetch(method&(~3));
	//
	//if (!pth)
	//{
	//	if (editstatus)
	//	{
	//		Bsprintf(ptempbuf, "pth==NULL! (bad pal?) pic=%d pal=%d", globalpicnum, globalpal);
	//		polymost_printext256(8, 8, editorcolors[15], editorcolors[5], ptempbuf, 0);
	//	}
	//	return;
	//}

	static int32_t fullbright_pass = 0;

	//if (pth->flags & PTH_HASFULLBRIGHT && indrawroomsandmasks && r_fullbrights)
	//{
	//	if (!fullbright_pass)
	//		fullbright_pass = 1;
	//	else if (fullbright_pass == 2)
	//		pth = pth->ofb;
	//}

	// If we aren't rendmode 3, we're in Polymer, which means this code is
	// used for rotatesprite only. Polymer handles all the material stuff,
	//// just submit the geometry and don't mess with textures.
	//if (getrendermode() == REND_POLYMOST)
	//{
	//	bglBindTexture(GL_TEXTURE_2D, pth ? pth->glpic : 0);
	//
	//	if (drawpoly_srepeat)
	//		bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//	if (drawpoly_trepeat)
	//		bglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//}
	//
	//// texture scale by parkar request
	//if (pth && pth->hicr && !drawingskybox && ((pth->hicr->scale.x != 1.0f) || (pth->hicr->scale.y != 1.0f)))
	//{
	//	bglMatrixMode(GL_TEXTURE);
	//	bglLoadIdentity();
	//	bglScalef(pth->hicr->scale.x, pth->hicr->scale.y, 1.0f);
	//	bglMatrixMode(GL_MODELVIEW);
	//}

	//int32_t texunits = GL_TEXTURE0_ARB;

//#ifndef EDUKE32_GLES
//	// detail texture
//	pthtyp *detailpth = NULL;
//
//	if (r_detailmapping)
//	{
//		if (usehightile && !drawingskybox && hicfindsubst(globalpicnum, DETAILPAL) &&
//			(detailpth = texcache_fetch(globalpicnum, DETAILPAL, 0, method & (~3))) &&
//			detailpth->hicr && detailpth->hicr->palnum == DETAILPAL)
//		{
//			polymost_setupdetailtexture(++texunits, detailpth ? detailpth->glpic : 0);
//
//			bglMatrixMode(GL_TEXTURE);
//			bglLoadIdentity();
//
//			if (pth && pth->hicr && ((pth->hicr->scale.x != 1.0f) || (pth->hicr->scale.y != 1.0f)))
//				bglScalef(pth->hicr->scale.x, pth->hicr->scale.y, 1.0f);
//
//			if (detailpth && detailpth->hicr && ((detailpth->hicr->scale.x != 1.0f) || (detailpth->hicr->scale.y != 1.0f)))
//				bglScalef(detailpth->hicr->scale.x, detailpth->hicr->scale.y, 1.0f);
//
//			bglMatrixMode(GL_MODELVIEW);
//		}
//	}
//
//	// glow texture
//	pthtyp *glowpth = NULL;
//
//	if (r_glowmapping)
//	{
//		if (usehightile && !drawingskybox && hicfindsubst(globalpicnum, GLOWPAL) &&
//			(glowpth = texcache_fetch(globalpicnum, GLOWPAL, 0, method&(~3))) &&
//			glowpth->hicr && (glowpth->hicr->palnum == GLOWPAL))
//			polymost_setupglowtexture(++texunits, glowpth ? glowpth->glpic : 0);
//	}
//#endif

	vec2f_t hacksc = { 1.f, 1.f };

//	if (pth && (pth->flags & PTH_HIGHTILE))
//	{
//		hacksc = pth->scale;
//		tsiz = pth->siz;
//	}

	vec2_t tsiz2 = tsiz;

	//if (!glinfo.texnpot)
	//{
	//	for (tsiz2.x = 1; tsiz2.x < tsiz.x; tsiz2.x += tsiz2.x)
	//		; /* do nothing */
	//	for (tsiz2.y = 1; tsiz2.y < tsiz.y; tsiz2.y += tsiz2.y)
	//		; /* do nothing */
	//}

	//if ((!(method & 3)) && (!fullbright_pass))
	//{
	//	bglDisable(GL_BLEND);
	//	bglDisable(GL_ALPHA_TEST);
	//}
	//else
	//{
	//	float const al = waloff[globalpicnum] ? alphahackarray[globalpicnum] * (1.f / 255.f) ? alphahackarray[globalpicnum] * (1.f / 255.f) :
	//		(pth && pth->hicr && pth->hicr->alphacut >= 0.f ? pth->hicr->alphacut : 0.f) : 0.f;
	//
	//	bglAlphaFunc(GL_GREATER, al);
	//	bglEnable(GL_BLEND);
	//	bglEnable(GL_ALPHA_TEST);
	//}

	float pc[4];


	pc[0] = pc[1] = pc[2] = getshadefactor(globalshade);

	// spriteext full alpha control
	static float drawpoly_alpha = 0.0f; // !!!!HACK HACK!!!!!
	pc[3] = float_trans[method & 3] * (1.f - drawpoly_alpha);

	//if (pth)
	//{
	//	// tinting
	//	if (!(hictinting[globalpal].f & HICTINT_PRECOMPUTED))
	//	{
	//		if (pth->flags & PTH_HIGHTILE)
	//		{
	//			if (pth->palnum != globalpal || (pth->effects & HICTINT_IN_MEMORY) || (hictinting[globalpal].f & HICTINT_APPLYOVERALTPAL))
	//				hictinting_apply(pc, globalpal);
	//		}
	//		else if (hictinting[globalpal].f & HICTINT_USEONART)
	//			hictinting_apply(pc, globalpal);
	//	}
	//
	//	// global tinting
	//	if ((pth->flags & PTH_HIGHTILE) && have_basepal_tint())
	//		hictinting_apply(pc, MAXPALOOKUPS - 1);
	//}

	//Hack for walls&masked walls which use textures that are not a power of 2
	if ((pow2xsplit) && (tsiz.x != tsiz2.x))
	{
		vec3f_t const opxy[3] = { { py[1] - py[2], py[2] - py[0], py[0] - py[1] },
		{ px[2] - px[1], px[0] - px[2], px[1] - px[0] },
		{ px[0] - .5f, py[0] - .5f, 0 } };

		float const r = 1.f / (opxy[0].x*px[0] + opxy[0].y*px[1] + opxy[0].z*px[2]);

		float ngdx = (opxy[0].x*dd[0] + opxy[0].y*dd[1] + opxy[0].z*dd[2])*r,
			ngux = (opxy[0].x*uu[0] + opxy[0].y*uu[1] + opxy[0].z*uu[2])*r,
			ngvx = (opxy[0].x*vv[0] + opxy[0].y*vv[1] + opxy[0].z*vv[2])*r;

		float ngdy = (opxy[1].x*dd[0] + opxy[1].y*dd[1] + opxy[1].z*dd[2])*r,
			nguy = (opxy[1].x*uu[0] + opxy[1].y*uu[1] + opxy[1].z*uu[2])*r,
			ngvy = (opxy[1].x*vv[0] + opxy[1].y*vv[1] + opxy[1].z*vv[2])*r;

		float ngdo = dd[0] - opxy[2].x * ngdx - opxy[2].y * ngdy,
			nguo = uu[0] - opxy[2].x * ngux - opxy[2].y * nguy,
			ngvo = vv[0] - opxy[2].x * ngvx - opxy[2].y * ngvy;

		ngux *= hacksc.x; nguy *= hacksc.x; nguo *= hacksc.x;
		ngvx *= hacksc.y; ngvy *= hacksc.y; ngvo *= hacksc.y;

		float const uoffs = ((float)(tsiz2.x - tsiz.x) * 0.5f);

		ngux -= ngdx * uoffs;
		nguy -= ngdy * uoffs;
		nguo -= ngdo * uoffs;

		float du0, du1;

		//Find min&max u coordinates (du0...du1)
		for (int i = 0; i<npoints; ++i)
		{
			vec2f_t const o = { px[i], py[i] };
			float const f = (o.x*ngux + o.y*nguy + nguo) / (o.x*ngdx + o.y*ngdy + ngdo);
			if (!i) { du0 = du1 = f; continue; }
			if (f < du0) du0 = f;
			else if (f > du1) du1 = f;
		}

		float const rf = 1.0f / tsiz.x;

		int32_t ix0 = (int)floorf(du0 * rf);
		int32_t const ix1 = (int)floorf(du1 * rf);

		for (; ix0 <= ix1; ++ix0)
		{
			du0 = (float)(ix0 * tsiz.x);        // + uoffs;
			du1 = (float)((ix0 + 1) * tsiz.x);  // + uoffs;

			float duj = (px[0] * ngux + py[0] * nguy + nguo) / (px[0] * ngdx + py[0] * ngdy + ngdo);
			int i = 0, nn = 0;

			do
			{
				j = i + 1;

				if (j == npoints)
					j = 0;

				float const dui = duj;

				duj = (px[j] * ngux + py[j] * nguy + nguo) / (px[j] * ngdx + py[j] * ngdy + ngdo);

				if ((du0 <= dui) && (dui <= du1))
				{
					uu[nn] = px[i];
					vv[nn] = py[i];
					nn++;
				}

				//ox*(ngux-ngdx*du1) + oy*(nguy-ngdy*du1) + (nguo-ngdo*du1) = 0
				//(px[j]-px[i])*f + px[i] = ox
				//(py[j]-py[i])*f + py[i] = oy

				///Solve for f
				//((px[j]-px[i])*f + px[i])*(ngux-ngdx*du1) +
				//((py[j]-py[i])*f + py[i])*(nguy-ngdy*du1) + (nguo-ngdo*du1) = 0

#define DRAWPOLY_MATH_BULLSHIT(XXX)                                                                                \
do                                                                                                                 \
{                                                                                                                  \
    float const f = -(px[i] * (ngux - ngdx * XXX) + py[i] * (nguy - ngdy * XXX) + (nguo - ngdo * XXX)) /           \
        ((px[j] - px[i]) * (ngux - ngdx * XXX) + (py[j] - py[i]) * (nguy - ngdy * XXX));                           \
    uu[nn] = (px[j] - px[i]) * f + px[i];                                                                          \
    vv[nn] = (py[j] - py[i]) * f + py[i];                                                                          \
    nn++;                                                                                                          \
} while (0)

				if (duj <= dui)
				{
					if ((du1 < duj) != (du1 < dui)) DRAWPOLY_MATH_BULLSHIT(du1);
					if ((du0 < duj) != (du0 < dui)) DRAWPOLY_MATH_BULLSHIT(du0);
				}
				else
				{
					if ((du0 < duj) != (du0 < dui)) DRAWPOLY_MATH_BULLSHIT(du0);
					if ((du1 < duj) != (du1 < dui)) DRAWPOLY_MATH_BULLSHIT(du1);
				}

#undef DRAWPOLY_MATH_BULLSHIT

				i = j;
			} while (i);

			if (nn < 3) continue;

			vec2f_t const invtsiz2 = { 1.f / tsiz2.x, 1.f / tsiz2.y };

			drawPolyCmd->numVertexes = nn;

			for (i = 0; i<nn; i++)
			{
				drawPolyCmd->SetCurrentVertex(i);

				vec2f_t const o = { uu[i], vv[i] };

				float const dp = o.x*ngdx + o.y*ngdy + ngdo,
					up = o.x*ngux + o.y*nguy + nguo,
					vp = o.x*ngvx + o.y*ngvy + ngvo;

				float const r = 1.f / dp;

				drawPolyCmd->SetVertexColor(pc[0], pc[1], pc[2], pc[3]);
				drawPolyCmd->SetTextureCoordinate((up * r - du0 + uoffs) * invtsiz2.x, vp * r * invtsiz2.y);
				drawPolyCmd->SetPosition((o.x - ghalfx) * r * grhalfxdown10x, (ghoriz - o.y) * r * grhalfxdown10, r * (1.f / 1024.f));
			}
			
		}
	}
	else
	{
		vec2f_t const scale = { 1.f / tsiz2.x * hacksc.x, 1.f / tsiz2.y * hacksc.y };

		drawPolyCmd->numVertexes = npoints;
		for (int i = 0; i < npoints; i++)
		{
			float const r = 1.f / dd[i];

			drawPolyCmd->SetCurrentVertex(i);
			drawPolyCmd->SetTextureCoordinate(uu[i] * r * scale.x, vv[i] * r * scale.y);

			drawPolyCmd->SetPosition((px[i] - ghalfx) * r * grhalfxdown10x, (ghoriz - py[i]) * r * grhalfxdown10, r * (1.f / 1024.f));
		}
	}

	if (fullbright_pass == 1)
	{
		int32_t const shade = globalshade;

		globalshade = -128;
		fullbright_pass = 2;

		drawpoly(dpxy, n, method_);

		globalshade = shade;
		fullbright_pass = 0;
	}
#endif
}


static inline void vsp_finalize_init(int32_t const vcnt)
{
	for (int i = 0; i<vcnt; i++)
	{
		vsp[i].cy[1] = vsp[i + 1].cy[0]; vsp[i].ctag = i;
		vsp[i].fy[1] = vsp[i + 1].fy[0]; vsp[i].ftag = i;
		vsp[i].n = i + 1; vsp[i].p = i - 1;
		//        vsp[i].tag = -1;
	}
	vsp[vcnt - 1].n = 0; vsp[0].p = vcnt - 1;

	//VSPMAX-1 is dummy empty node
	for (int i = vcnt; i<VSPMAX; i++) { vsp[i].n = i + 1; vsp[i].p = i - 1; }
	vsp[VSPMAX - 1].n = vcnt; vsp[vcnt].p = VSPMAX - 1;
}

#define COMBINE_STRIPS

#ifdef COMBINE_STRIPS
static inline void vsdel(int32_t const i)
{
	//Delete i
	int const pi = vsp[i].p;
	int const ni = vsp[i].n;

	vsp[ni].p = pi;
	vsp[pi].n = ni;

	//Add i to empty list
	vsp[i].n = vsp[VSPMAX - 1].n;
	vsp[i].p = VSPMAX - 1;
	vsp[vsp[VSPMAX - 1].n].p = i;
	vsp[VSPMAX - 1].n = i;
}
#endif

static inline int32_t vsinsaft(int32_t const i)
{
	//i = next element from empty list
	int32_t const r = vsp[VSPMAX - 1].n;
	vsp[vsp[r].n].p = VSPMAX - 1;
	vsp[VSPMAX - 1].n = vsp[r].n;

	vsp[r] = vsp[i]; //copy i to r

					 //insert r after i
	vsp[r].p = i; vsp[r].n = vsp[i].n;
	vsp[vsp[i].n].p = r; vsp[i].n = r;

	return r;
}

static int32_t domostpolymethod = 0;

#define DOMOST_OFFSET .01f

void Build3DBoardPolymost::AddUniqueSector(short sectnum)
{
	if (sectnum < 0)
		return;

	visibleSectorList[sectnum] = 1;
}

void Build3DBoardPolymost::domost(float x0, float y0, float x1, float y1, short sectorNum)
{
	int32_t const dir = (x0 < x1);

	AddUniqueSector(sectorNum);

	if (dir) //clip dmost (floor)
	{
		y0 -= DOMOST_OFFSET;
		y1 -= DOMOST_OFFSET;
	}
	else //clip umost (ceiling)
	{
		if (x0 == x1) return;
		swapfloat(&x0, &x1);
		swapfloat(&y0, &y1);
		y0 += DOMOST_OFFSET;
		y1 += DOMOST_OFFSET; //necessary?
	}

	vec2f_t const dm0 = { x0, y0 };
	vec2f_t const dm1 = { x1, y1 };

	float const slop = (dm1.y - dm0.y) / (dm1.x - dm0.x);

	//drawpoly_alpha = 0.f;

	vec2f_t n0, n1;
	float spx[4];
	int32_t  spt[4];

	for (int newi, i = vsp[0].n; i; i = newi)
	{
		newi = vsp[i].n; n0.x = vsp[i].x; n1.x = vsp[newi].x;

		if ((dm0.x >= n1.x) || (n0.x >= dm1.x) || (vsp[i].ctag <= 0)) continue;

		float const dx = n1.x - n0.x;
		float const cy[2] = { vsp[i].cy[0], vsp[i].fy[0] },
			cv[2] = { vsp[i].cy[1] - cy[0], vsp[i].fy[1] - cy[1] };

		int scnt = 0;

		//Test if left edge requires split (dm0.x,dm0.y) (nx0,cy(0)),<dx,cv(0)>
		if ((dm0.x > n0.x) && (dm0.x < n1.x))
		{
			float const t = (dm0.x - n0.x)*cv[dir] - (dm0.y - cy[dir])*dx;
			if (((!dir) && (t < 0.f)) || ((dir) && (t > 0.f)))
			{
				spx[scnt] = dm0.x; spt[scnt] = -1; scnt++;
			}
		}

		//Test for intersection on umost (0) and dmost (1)

		float const d[2] = { ((dm0.y - dm1.y) * dx) - ((dm0.x - dm1.x) * cv[0]),
			((dm0.y - dm1.y) * dx) - ((dm0.x - dm1.x) * cv[1]) };

		float const n[2] = { ((dm0.y - cy[0]) * dx) - ((dm0.x - n0.x) * cv[0]),
			((dm0.y - cy[1]) * dx) - ((dm0.x - n0.x) * cv[1]) };

		float const fnx[2] = { dm0.x + ((n[0] / d[0]) * (dm1.x - dm0.x)),
			dm0.x + ((n[1] / d[1]) * (dm1.x - dm0.x)) };

		if ((Bfabsf(d[0]) > Bfabsf(n[0])) && (d[0] * n[0] >= 0.f) && (fnx[0] > n0.x) && (fnx[0] < n1.x))
			spx[scnt] = fnx[0], spt[scnt++] = 0;

		if ((Bfabsf(d[1]) > Bfabsf(n[1])) && (d[1] * n[1] >= 0.f) && (fnx[1] > n0.x) && (fnx[1] < n1.x))
			spx[scnt] = fnx[1], spt[scnt++] = 1;

		//Nice hack to avoid full sort later :)
		if ((scnt >= 2) && (spx[scnt - 1] < spx[scnt - 2]))
		{
			swapfloat(&spx[scnt - 1], &spx[scnt - 2]);
			swaplong(&spt[scnt - 1], &spt[scnt - 2]);
		}

		//Test if right edge requires split
		if ((dm1.x > n0.x) && (dm1.x < n1.x))
		{
			const float t = (dm1.x - n0.x)*cv[dir] - (dm1.y - cy[dir])*dx;
			if (((!dir) && (t < 0)) || ((dir) && (t > 0)))
			{
				spx[scnt] = dm1.x; spt[scnt] = -1; scnt++;
			}
		}

		vsp[i].tag = vsp[newi].tag = -1;

		float const rdx = 1.f / dx;

		for (int z = 0, vcnt = 0; z <= scnt; z++, i = vcnt)
		{
			float t;

			if (z == scnt)
				goto skip;

			t = (spx[z] - n0.x)*rdx;
			vcnt = vsinsaft(i);
			vsp[i].cy[1] = t*cv[0] + cy[0];
			vsp[i].fy[1] = t*cv[1] + cy[1];
			vsp[vcnt].x = spx[z];
			vsp[vcnt].cy[0] = vsp[i].cy[1];
			vsp[vcnt].fy[0] = vsp[i].fy[1];
			vsp[vcnt].tag = spt[z];

		skip:;
			int32_t const ni = vsp[i].n; if (!ni) continue; //this 'if' fixes many bugs!
			float const dx0 = vsp[i].x; if (dm0.x > dx0) continue;
			float const dx1 = vsp[ni].x; if (dm1.x < dx1) continue;
			n0.y = (dx0 - dm0.x)*slop + dm0.y;
			n1.y = (dx1 - dm0.x)*slop + dm0.y;

			//      dx0           dx1
			//       ~             ~
			//----------------------------
			//     t0+=0         t1+=0
			//   vsp[i].cy[0]  vsp[i].cy[1]
			//============================
			//     t0+=1         t1+=3
			//============================
			//   vsp[i].fy[0]    vsp[i].fy[1]
			//     t0+=2         t1+=6
			//
			//     ny0 ?         ny1 ?

			int k = 4;

			if ((vsp[i].tag == 0) || (n0.y <= vsp[i].cy[0] + DOMOST_OFFSET)) k--;
			if ((vsp[i].tag == 1) || (n0.y >= vsp[i].fy[0] - DOMOST_OFFSET)) k++;
			if ((vsp[ni].tag == 0) || (n1.y <= vsp[i].cy[1] + DOMOST_OFFSET)) k -= 3;
			if ((vsp[ni].tag == 1) || (n1.y >= vsp[i].fy[1] - DOMOST_OFFSET)) k += 3;

			if (!dir)
			{
				switch (k)
				{
				case 4:
				case 5:
				case 7:
				{
					vec2f_t const dpxy[4] = {
						{ dx0, vsp[i].cy[0] },{ dx1, vsp[i].cy[1] },{ dx1, n1.y },{ dx0, n0.y }
					};

					vsp[i].cy[0] = n0.y;
					vsp[i].cy[1] = n1.y;
					vsp[i].ctag = gtag;
					drawpoly(dpxy, 4, domostpolymethod);
				}
				break;
				case 1:
				case 2:
				{
					vec2f_t const dpxy[3] = { { dx0, vsp[i].cy[0] },{ dx1, vsp[i].cy[1] },{ dx0, n0.y } };

					vsp[i].cy[0] = n0.y;
					vsp[i].ctag = gtag;
					drawpoly(dpxy, 3, domostpolymethod);
				}
				break;
				case 3:
				case 6:
				{
					vec2f_t const dpxy[3] = { { dx0, vsp[i].cy[0] },{ dx1, vsp[i].cy[1] },{ dx1, n1.y } };

					vsp[i].cy[1] = n1.y;
					vsp[i].ctag = gtag;
					drawpoly(dpxy, 3, domostpolymethod);
				}
				break;
				case 8:
				{
					vec2f_t const dpxy[4] = {
						{ dx0, vsp[i].cy[0] },{ dx1, vsp[i].cy[1] },{ dx1, vsp[i].fy[1] },{ dx0, vsp[i].fy[0] }
					};

					vsp[i].ctag = vsp[i].ftag = -1;
					drawpoly(dpxy, 4, domostpolymethod);
				}
				default: break;
				}
			}
			else
			{
				switch (k)
				{
				case 4:
				case 3:
				case 1:
				{
					vec2f_t const dpxy[4] = {
						{ dx0, n0.y },{ dx1, n1.y },{ dx1, vsp[i].fy[1] },{ dx0, vsp[i].fy[0] }
					};

					vsp[i].fy[0] = n0.y;
					vsp[i].fy[1] = n1.y;
					vsp[i].ftag = gtag;
					drawpoly(dpxy, 4, domostpolymethod);
				}
				break;
				case 7:
				case 6:
				{
					vec2f_t const dpxy[3] = { { dx0, n0.y },{ dx1, vsp[i].fy[1] },{ dx0, vsp[i].fy[0] } };

					vsp[i].fy[0] = n0.y;
					vsp[i].ftag = gtag;
					drawpoly(dpxy, 3, domostpolymethod);
				}
				break;
				case 5:
				case 2:
				{
					vec2f_t const dpxy[3] = { { dx0, vsp[i].fy[0] },{ dx1, n1.y },{ dx1, vsp[i].fy[1] } };

					vsp[i].fy[1] = n1.y;
					vsp[i].ftag = gtag;
					drawpoly(dpxy, 3, domostpolymethod);
				}
				break;
				case 0:
				{
					vec2f_t const dpxy[4] = { { dx0, vsp[i].cy[0] },{ dx1, vsp[i].cy[1] },{ dx1, vsp[i].fy[1] },{ dx0, vsp[i].fy[0] } };
					vsp[i].ctag = vsp[i].ftag = -1;
					drawpoly(dpxy, 4, domostpolymethod);
				}
				default:
					break;
				}
			}
		}
	}

	gtag++;

	//Combine neighboring vertical strips with matching collinear top&bottom edges
	//This prevents x-splits from propagating through the entire scan
#ifdef COMBINE_STRIPS
	int i = vsp[0].n;

	while (i)
	{
		if ((vsp[i].cy[0] >= vsp[i].fy[0]) && (vsp[i].cy[1] >= vsp[i].fy[1]))
			vsp[i].ctag = vsp[i].ftag = -1;

		int const ni = vsp[i].n;

		if ((vsp[i].ctag == vsp[ni].ctag) && (vsp[i].ftag == vsp[ni].ftag))
		{
			vsp[i].cy[1] = vsp[ni].cy[1];
			vsp[i].fy[1] = vsp[ni].fy[1];
			vsdel(ni);
		}
		else i = ni;
	}
#endif
}

#define POINT2(i) (wall[wall[i].point2])

//void polymost_editorfunc(void)
//{
//	const float ratio = (r_usenewaspect ? (fxdim / fydim) / (320.f / 240.f) : 1.f)  * (1.f / get_projhack_ratio());
//
//	vec3f_t tvect = { (searchx - ghalfx) * ratio, (searchy - ghoriz) * ratio, ghalfx };
//
//	//Tilt rotation
//	vec3f_t o = { tvect.x * gctang + tvect.y * gstang, tvect.y * gctang - tvect.x * gstang, tvect.z };
//
//	//Up/down rotation
//	tvect.x = o.z*gchang - o.y*gshang;
//	tvect.y = o.x;
//	tvect.z = o.y*gchang + o.z*gshang;
//
//	//Standard Left/right rotation
//	vec3_t v = { Blrintf(tvect.x * fcosglobalang - tvect.y * fsinglobalang),
//		Blrintf(tvect.x * fsinglobalang + tvect.y * fcosglobalang), Blrintf(tvect.z * 16384.f) };
//
//	vec3_t vect = { globalposx, globalposy, globalposz };
//
//	hitdata_t *hit = &polymost_hitdata;
//
//	hitallsprites = 1;
//
//	hitscan((const vec3_t *)&vect, globalcursectnum, //Start position
//		v.x >> 10, v.y >> 10, v.z >> 6, hit, 0xffff0030);
//
//	if (hit->sect != -1) // if hitsect is -1, hitscan overflowed somewhere
//	{
//		int32_t cz, fz;
//		getzsofslope(hit->sect, hit->pos.x, hit->pos.y, &cz, &fz);
//		hitallsprites = 0;
//
//		searchsector = hit->sect;
//		if (hit->pos.z<cz) searchstat = 1;
//		else if (hit->pos.z>fz) searchstat = 2;
//		else if (hit->wall >= 0)
//		{
//			searchbottomwall = searchwall = hit->wall; searchstat = 0;
//			if (wall[hit->wall].nextwall >= 0)
//			{
//				getzsofslope(wall[hit->wall].nextsector, hit->pos.x, hit->pos.y, &cz, &fz);
//				if (hit->pos.z > fz)
//				{
//					searchisbottom = 1;
//					if (wall[hit->wall].cstat & 2) //'2' bottoms of walls
//						searchbottomwall = wall[hit->wall].nextwall;
//				}
//				else
//				{
//					searchisbottom = 0;
//					if ((hit->pos.z > cz) && (wall[hit->wall].cstat&(16 + 32))) //masking or 1-way
//						searchstat = 4;
//				}
//			}
//		}
//		else if (hit->sprite >= 0) { searchwall = hit->sprite; searchstat = 3; }
//		else
//		{
//			getzsofslope(hit->sect, hit->pos.x, hit->pos.y, &cz, &fz);
//			if ((hit->pos.z << 1) < cz + fz) searchstat = 1; else searchstat = 2;
//			//if (vz < 0) searchstat = 1; else searchstat = 2; //Won't work for slopes :/
//		}
//
//		if (preview_mouseaim)
//		{
//			if (spritesortcnt == MAXSPRITESONSCREEN)
//				spritesortcnt--;
//
//			tspritetype *tsp = &tsprite[spritesortcnt];
//			double dadist, x, y, z;
//			Bmemcpy(tsp, &hit->pos, sizeof(vec3_t));
//			x = tsp->x - globalposx; y = tsp->y - globalposy; z = (tsp->z - globalposz) / 16.0;
//			dadist = Bsqrt(x*x + y*y + z*z);
//			tsp->sectnum = hit->sect;
//			tsp->picnum = 2523;  // CROSSHAIR
//			tsp->cstat = 128;
//
//			if (hit->wall != -1)
//			{
//				tsp->cstat |= 16;
//				int const ang = getangle(wall[hit->wall].x - POINT2(hit->wall).x, wall[hit->wall].y - POINT2(hit->wall).y);
//				tsp->ang = ang + 512;
//
//				vec2_t const offs = { sintable[(ang + 1024) & 2047] >> 11,
//					sintable[(ang + 512) & 2047] >> 11 };
//
//				tsp->x -= offs.x;
//				tsp->y -= offs.y;
//
//			}
//			else if (hit->sprite == -1 && (hit->pos.z == sector[hit->sect].floorz || hit->pos.z == sector[hit->sect].ceilingz))
//			{
//				tsp->cstat = 32;
//				tsp->ang = getangle(hit->pos.x - globalposx, hit->pos.y - globalposy);
//			}
//			else if (hit->sprite >= 0)
//			{
//				if (sprite[hit->sprite].cstat & 16)
//				{
//					tsp->cstat |= 16;
//					tsp->ang = sprite[hit->sprite].ang;
//				}
//
//				else tsp->ang = (globalang + 1024) & 2047;
//
//				vec2_t const offs = { sintable[(tsp->ang + 1536) & 2047] >> 11,
//					sintable[(tsp->ang + 1024) & 2047] >> 11 };
//
//				tsp->x -= offs.x;
//				tsp->y -= offs.y;
//			}
//			static int lastupdate = 0;
//			static int shd = 30;
//			static int shdinc = 1;
//
//			if (totalclock > lastupdate)
//			{
//				shd += shdinc;
//				if (shd >= 30 || shd <= 0)
//				{
//					shdinc = -shdinc;
//					shd += shdinc;
//				}
//				lastupdate = totalclock + 3;
//			}
//
//			tsp->shade = 30 - shd;
//			tsp->owner = MAXSPRITES - 1;
//			tsp->xrepeat = tsp->yrepeat = min(max(1, (int32_t)(dadist*((double)(shd * 3) / 3200.0))), 255);
//			sprite[tsp->owner].xoffset = sprite[tsp->owner].yoffset = 0;
//			tspriteptr[spritesortcnt++] = tsp;
//		}
//
//		if ((searchstat == 1 || searchstat == 2) && searchsector >= 0)
//		{
//			vec2_t const scrv = { (v.x >> 12), (v.y >> 12) };
//			vec2_t const scrv_r = { scrv.y, -scrv.x };
//			walltype const * const wal = &wall[sector[searchsector].wallptr];
//			uint64_t bestwdistsq = 0x7fffffff;
//			int32_t bestk = -1;
//
//			for (int32_t k = 0; k < sector[searchsector].wallnum; k++)
//			{
//				vec2_t const w1 = { wal[k].x, wal[k].y };
//				vec2_t const w2 = { wall[wal[k].point2].x, wall[wal[k].point2].y };
//				vec2_t const w21 = { w1.x - w2.x, w1.y - w2.y };
//				vec2_t const pw1 = { w1.x - hit->pos.x, w1.y - hit->pos.y };
//				vec2_t const pw2 = { w2.x - hit->pos.x, w2.y - hit->pos.y };
//				float w1d = (float)(scrv_r.x * pw1.x + scrv_r.y * pw1.y);
//				float w2d = (float)-(scrv_r.x * pw2.x + scrv_r.y * pw2.y);
//
//				if ((w1d == 0 && w2d == 0) || (w1d < 0 || w2d < 0))
//					continue;
//
//				vec2_t const ptonline = { (int32_t)(w2.x + (w2d / (w1d + w2d)) * w21.x),
//					(int32_t)(w2.y + (w2d / (w1d + w2d)) * w21.y) };
//
//				vec2_t const scrp = { ptonline.x - vect.x, ptonline.y - vect.y };
//
//				if (scrv.x * scrp.x + scrv.y * scrp.y <= 0)
//					continue;
//
//				int64_t const t1 = scrp.x;
//				int64_t const t2 = scrp.y;
//
//				uint64_t const wdistsq = t1 * t1 + t2 * t2;
//
//				if (wdistsq < bestwdistsq)
//				{
//					bestk = k;
//					bestwdistsq = wdistsq;
//				}
//			}
//
//			if (bestk >= 0)
//				searchwall = sector[searchsector].wallptr + bestk;
//		}
//	}
//	searchit = 0;
//}

// variables that are set to ceiling- or floor-members, depending
// on which one is processed right now
static int32_t global_cf_z;
static float global_cf_xpanning, global_cf_ypanning, global_cf_heinum;
static int32_t global_cf_shade, global_cf_pal, global_cf_fogpal;
static int32_t(*global_getzofslope_func)(int16_t, int32_t, int32_t);

void Build3DBoardPolymost::internal_nonparallaxed(vec2f_t n0, vec2f_t n1, float ryp0, float ryp1, float x0, float x1, float y0, float y1, int32_t sectnum)
{
	int const have_floor = sectnum & MAXSECTORS;
	sectnum &= ~MAXSECTORS;
	tsectortype const * const sec = (tsectortype *)&sector[sectnum];

	// comments from floor code:
	//(singlobalang/-16384*(sx-ghalfx) + 0*(sy-ghoriz) + (cosviewingrangeglobalang/16384)*ghalfx)*d + globalposx    = u*16
	//(cosglobalang/ 16384*(sx-ghalfx) + 0*(sy-ghoriz) + (sinviewingrangeglobalang/16384)*ghalfx)*d + globalposy    = v*16
	//(                  0*(sx-ghalfx) + 1*(sy-ghoriz) + (                             0)*ghalfx)*d + globalposz/16 = (sec->floorz/16)

	float ft[4] = { fglobalposx, fglobalposy, fcosglobalang, fsinglobalang };

	if (globalorientation & 64)
	{
		//relative alignment
		vec2f_t fxy = { (float)(wall[wall[sec->wallptr].point2].x - wall[sec->wallptr].x),
			(float)(wall[wall[sec->wallptr].point2].y - wall[sec->wallptr].y) };

		float r = polymost_invsqrt_approximation(fxy.x * fxy.x + fxy.y * fxy.y);

		fxy.x *= r;
		fxy.y *= r;

		ft[0] = ((float)(globalposx - wall[sec->wallptr].x)) * fxy.x + ((float)(globalposy - wall[sec->wallptr].y)) * fxy.y;
		ft[1] = ((float)(globalposy - wall[sec->wallptr].y)) * fxy.x - ((float)(globalposx - wall[sec->wallptr].x)) * fxy.y;
		ft[2] = fcosglobalang * fxy.x + fsinglobalang * fxy.y;
		ft[3] = fsinglobalang * fxy.x - fcosglobalang * fxy.y;

		globalorientation ^= (!(globalorientation & 4)) ? 32 : 16;
	}

	xtex.d = 0;
	ytex.d = gxyaspect;

	if (!(globalorientation & 2) && global_cf_z - globalposz)  // PK 2012: don't allow div by zero
		ytex.d /= (double)(global_cf_z - globalposz);

	otex.d = -ghoriz * ytex.d;

	if (globalorientation & 8)
	{
		ft[0] *= (1.f / 8.f);
		ft[1] *= -(1.f / 8.f);
		ft[2] *= (1.f / 2097152.f);
		ft[3] *= (1.f / 2097152.f);
	}
	else
	{
		ft[0] *= (1.f / 16.f);
		ft[1] *= -(1.f / 16.f);
		ft[2] *= (1.f / 4194304.f);
		ft[3] *= (1.f / 4194304.f);
	}

	xtex.u = ft[3] * -(1.f / 65536.f) * (double)viewingrange;
	xtex.v = ft[2] * -(1.f / 65536.f) * (double)viewingrange;
	ytex.u = ft[0] * ytex.d;
	ytex.v = ft[1] * ytex.d;
	otex.u = ft[0] * otex.d;
	otex.v = ft[1] * otex.d;
	otex.u += (ft[2] - xtex.u) * ghalfx;
	otex.v -= (ft[3] + xtex.v) * ghalfx;

	//Texture flipping
	if (globalorientation & 4)
	{
		swap64bit(&xtex.u, &xtex.v);
		swap64bit(&ytex.u, &ytex.v);
		swap64bit(&otex.u, &otex.v);
	}

	if (globalorientation & 16) { xtex.u = -xtex.u; ytex.u = -ytex.u; otex.u = -otex.u; }
	if (globalorientation & 32) { xtex.v = -xtex.v; ytex.v = -ytex.v; otex.v = -otex.v; }

	//Texture panning
	vec2f_t fxy = { global_cf_xpanning * ((float)(1 << (picsiz[globalpicnum] & 15))) * (1.0f / 256.f),
		global_cf_ypanning * ((float)(1 << (picsiz[globalpicnum] >> 4))) * (1.0f / 256.f) };

	if ((globalorientation&(2 + 64)) == (2 + 64)) //Hack for panning for slopes w/ relative alignment
	{
		float r = global_cf_heinum * (1.0f / 4096.f);
		r = polymost_invsqrt_approximation(r * r + 1);

		if (!(globalorientation & 4))
			fxy.y *= r;
		else
			fxy.x *= r;
	}
	ytex.u += ytex.d*fxy.x; otex.u += otex.d*fxy.x;
	ytex.v += ytex.d*fxy.y; otex.v += otex.d*fxy.y;

	if (globalorientation & 2) //slopes
	{
		//Pick some point guaranteed to be not collinear to the 1st two points
		vec2f_t const oxy = { n0.x + (n1.y - n0.y), n0.y + (n0.x - n1.x) };

		float const ox2 = (oxy.y - fglobalposy) * gcosang - (oxy.x - fglobalposx) * gsinang;
		float oy2 = 1.f / ((oxy.x - fglobalposx) * gcosang2 + (oxy.y - fglobalposy) * gsinang2);

		double const px[3] = { x0, x1, ghalfx * ox2 * oy2 + ghalfx };

		oy2 *= gyxscale;

		double py[3] = { ryp0 + (double)ghoriz, ryp1 + (double)ghoriz, oy2 + (double)ghoriz };

		vec3f_t const duv[3] = {
			{ (float)(px[0] * xtex.d + py[0] * ytex.d + otex.d),
			(float)(px[0] * xtex.u + py[0] * ytex.u + otex.u),
			(float)(px[0] * xtex.v + py[0] * ytex.v + otex.v)
			},
			{ (float)(px[1] * xtex.d + py[1] * ytex.d + otex.d),
			(float)(px[1] * xtex.u + py[1] * ytex.u + otex.u),
			(float)(px[1] * xtex.v + py[1] * ytex.v + otex.v)
			},
			{ (float)(px[2] * xtex.d + py[2] * ytex.d + otex.d),
			(float)(px[2] * xtex.u + py[2] * ytex.u + otex.u),
			(float)(px[2] * xtex.v + py[2] * ytex.v + otex.v)
			}
		};

		py[0] = y0;
		py[1] = y1;
		py[2] = (double)((float)(global_getzofslope_func(sectnum, (int)oxy.x, (int)oxy.y) - globalposz) * oy2 + ghoriz);

		vec3f_t oxyz[2] = { { (float)(py[1] - py[2]), (float)(py[2] - py[0]), (float)(py[0] - py[1]) },
		{ (float)(px[2] - px[1]), (float)(px[0] - px[2]), (float)(px[1] - px[0]) } };

		float const r = 1.f / (oxyz[0].x * px[0] + oxyz[0].y * px[1] + oxyz[0].z * px[2]);

		xtex.d = (oxyz[0].x * duv[0].d + oxyz[0].y * duv[1].d + oxyz[0].z * duv[2].d) * r;
		xtex.u = (oxyz[0].x * duv[0].u + oxyz[0].y * duv[1].u + oxyz[0].z * duv[2].u) * r;
		xtex.v = (oxyz[0].x * duv[0].v + oxyz[0].y * duv[1].v + oxyz[0].z * duv[2].v) * r;

		ytex.d = (oxyz[1].x * duv[0].d + oxyz[1].y * duv[1].d + oxyz[1].z * duv[2].d) * r;
		ytex.u = (oxyz[1].x * duv[0].u + oxyz[1].y * duv[1].u + oxyz[1].z * duv[2].u) * r;
		ytex.v = (oxyz[1].x * duv[0].v + oxyz[1].y * duv[1].v + oxyz[1].z * duv[2].v) * r;

		otex.d = duv[0].d - px[0] * xtex.d - py[0] * ytex.d;
		otex.u = duv[0].u - px[0] * xtex.u - py[0] * ytex.u;
		otex.v = duv[0].v - px[0] * xtex.v - py[0] * ytex.v;

		if (globalorientation & 64) //Hack for relative alignment on slopes
		{
			float r = global_cf_heinum * (1.0f / 4096.f);
			r = Bsqrtf(r*r + 1);
			if (!(globalorientation & 4)) { xtex.v *= r; ytex.v *= r; otex.v *= r; }
			else { xtex.u *= r; ytex.u *= r; otex.u *= r; }
		}
	}

	domostpolymethod = (globalorientation >> 7) & 3;

	pow2xsplit = 0;
//	drawpoly_alpha = 0.f;

	//if (!nofog) calc_and_apply_fog(globalpicnum, fogpal_shade(sec, global_cf_shade), sec->visibility,
	//	POLYMOST_CHOOSE_FOG_PAL(global_cf_fogpal, global_cf_pal));

	if (have_floor)
	{
		if (globalposz > getflorzofslope(sectnum, globalposx, globalposy))
			domostpolymethod = -1; //Back-face culling

		domost(x0, y0, x1, y1, sectnum); //flor
	}
	else
	{
		if (globalposz < getceilzofslope(sectnum, globalposx, globalposy))
			domostpolymethod = -1; //Back-face culling

		domost(x1, y1, x0, y0, sectnum); //ceil
	}

	AddUniqueSector(sectnum);

	domostpolymethod = 0;
}

void Build3DBoardPolymost::calc_ypanning(int32_t refposz, float ryp0, float ryp1, float x0, float x1, uint8_t ypan, uint8_t yrepeat, int32_t dopancor)
{
	float const t0 = ((float)(refposz - globalposz))*ryp0 + ghoriz;
	float const t1 = ((float)(refposz - globalposz))*ryp1 + ghoriz;
	float t = ((xtex.d*x0 + otex.d) * (float)yrepeat) / ((x1 - x0) * ryp0 * 2048.f);
	int i = (1 << (picsiz[globalpicnum] >> 4));
	if (i < tilesiz[globalpicnum].y) i <<= 1;

#ifdef NEW_MAP_FORMAT
	if (g_loadedMapVersion >= 10)
		i = tilesiz[globalpicnum].y;
	else
#endif
		if (dopancor)
		{
			// Carry out panning "correction" to make it look like classic in some
			// cases, but failing in the general case.
			int32_t yoffs = Blrintf((i - tilesiz[globalpicnum].y)*(255.f / i));

			if (ypan > 256 - yoffs)
				ypan -= yoffs;
		}

	float const fy = (float)(ypan * i) * (1.f / 256.f);
	xtex.v = (t0 - t1)*t;
	ytex.v = (x1 - x0)*t;
	otex.v = -xtex.v*x0 - ytex.v*t0 + fy*otex.d; xtex.v += fy*xtex.d; ytex.v += fy*ytex.d;
}

int32_t Build3DBoardPolymost::testvisiblemost(float const x0, float const x1)
{
	for (int i = vsp[0].n, newi; i; i = newi)
	{
		newi = vsp[i].n;
		if ((x0 < vsp[newi].x) && (vsp[i].x < x1) && (vsp[i].ctag >= 0))
			return 1;
	}
	return 0;
}

int Build3DBoardPolymost::getclosestpointonwall(vec2_t const * const pos, int32_t dawall, vec2_t * const n)
{
	vec2_t const w = { wall[dawall].x, wall[dawall].y };
	vec2_t const d = { POINT2(dawall).x - w.x, POINT2(dawall).y - w.y };
	int64_t i = d.x * (pos->x - w.x) + d.y * (pos->y - w.y);

	if (i < 0)
		return 1;

	int64_t const j = d.x * d.x + d.y * d.y;

	if (i > j)
		return 1;

	i = tabledivide64((i << 15), j) << 15;

	n->x = w.x + ((d.x * i) >> 30);
	n->y = w.y + ((d.y * i) >> 30);

	return 0;
}

void Build3DBoardPolymost::drawalls(int32_t const bunch)
{
//	drawpoly_alpha = 0.f;

	int32_t const sectnum = thesector[bunchfirst[bunch]];
	tsectortype const * const sec = (tsectortype *)&sector[sectnum];

	AddUniqueSector(sectnum);

	//DRAW WALLS SECTION!
	for (int z = bunchfirst[bunch]; z >= 0; z = bunchp2[z])
	{
		int32_t const wallnum = thewall[z];

#ifdef YAX_ENABLE
		if (yax_nomaskpass == 1 && yax_isislandwall(wallnum, !yax_globalcf) && (yax_nomaskdidit = 1))
			continue;
#endif

		twalltype * const wal = (twalltype *)&wall[wallnum], *wal2 = (twalltype *)&wall[wal->point2];
		int32_t const nextsectnum = wal->nextsector;
		tsectortype * const nextsec = nextsectnum >= 0 ? (tsectortype *)&sector[nextsectnum] : NULL;

		AddUniqueSector(nextsectnum);

		//Offset&Rotate 3D coordinates to screen 3D space
		vec2f_t walpos = { (float)(wal->x - globalposx), (float)(wal->y - globalposy) };

		vec2f_t p0 = { walpos.y * gcosang - walpos.x * gsinang, walpos.x * gcosang2 + walpos.y * gsinang2 };
		vec2f_t const op0 = p0;

		walpos.x = (float)(wal2->x - globalposx); walpos.y = (float)(wal2->y - globalposy);

		vec2f_t p1 = { walpos.y * gcosang - walpos.x * gsinang, walpos.x * gcosang2 + walpos.y * gsinang2 };

		//Clip to close parallel-screen plane

		vec2f_t n0, n1;
		float t0, t1;

		if (p0.y < SCISDIST)
		{
			if (p1.y < SCISDIST) continue;
			t0 = (SCISDIST - p0.y) / (p1.y - p0.y); p0.x = (p1.x - p0.x)*t0 + p0.x; p0.y = SCISDIST;
			n0.x = (wal2->x - wal->x)*t0 + wal->x;
			n0.y = (wal2->y - wal->y)*t0 + wal->y;
		}
		else { t0 = 0.f; n0.x = (float)wal->x; n0.y = (float)wal->y; }
		if (p1.y < SCISDIST)
		{
			t1 = (SCISDIST - op0.y) / (p1.y - op0.y); p1.x = (p1.x - op0.x)*t1 + op0.x; p1.y = SCISDIST;
			n1.x = (wal2->x - wal->x)*t1 + wal->x;
			n1.y = (wal2->y - wal->y)*t1 + wal->y;
		}
		else { t1 = 1.f; n1.x = (float)wal2->x; n1.y = (float)wal2->y; }

		float ryp0 = 1.f / p0.y, ryp1 = 1.f / p1.y;

		//Generate screen coordinates for front side of wall
		float const x0 = ghalfx*p0.x*ryp0 + ghalfx, x1 = ghalfx*p1.x*ryp1 + ghalfx;

		if (x1 <= x0) continue;

		ryp0 *= gyxscale; ryp1 *= gyxscale;

		int32_t cz, fz;

		getzsofslope(sectnum,/*Blrintf(nx0)*/(int)n0.x,/*Blrintf(ny0)*/(int)n0.y, &cz, &fz);
		float const cy0 = ((float)(cz - globalposz))*ryp0 + ghoriz, fy0 = ((float)(fz - globalposz))*ryp0 + ghoriz;

		getzsofslope(sectnum,/*Blrintf(nx1)*/(int)n1.x,/*Blrintf(ny1)*/(int)n1.y, &cz, &fz);
		float const cy1 = ((float)(cz - globalposz))*ryp1 + ghoriz, fy1 = ((float)(fz - globalposz))*ryp1 + ghoriz;

		// Floor

		globalpicnum = sec->floorpicnum;
		globalshade = sec->floorshade;
		globalpal = sec->floorpal;
		globalorientation = sec->floorstat;
		globvis = (sector[sectnum].visibility != 0) ?
			mulscale4(globalcisibility, (uint8_t)(sector[sectnum].visibility + 16)) :
			globalcisibility;

		DO_TILE_ANIM(globalpicnum, sectnum);

		int32_t dapskybits;
		int8_t const * dapskyoff = getpsky(globalpicnum, NULL, &dapskybits);

		global_cf_fogpal = sec->fogpal;
		global_cf_shade = sec->floorshade, global_cf_pal = sec->floorpal; global_cf_z = sec->floorz;  // REFACT
		global_cf_xpanning = sec->floorxpanning; global_cf_ypanning = sec->floorypanning, global_cf_heinum = sec->floorheinum;
		global_getzofslope_func = &getflorzofslope;

		if (!(globalorientation & 1))
		{
#ifdef YAX_ENABLE
			if (globalposz <= sec->floorz || yax_getbunch(sectnum, YAX_FLOOR) < 0 || yax_getnextwall(wallnum, YAX_FLOOR) >= 0)
#endif
				internal_nonparallaxed(n0, n1, ryp0, ryp1, x0, x1, fy0, fy1, sectnum | MAXSECTORS);
		}
		else if ((nextsectnum < 0) || (!(sector[nextsectnum].floorstat & 1)))
		{
			//Parallaxing sky... hacked for Ken's mountain texture
			//if (!nofog) calc_and_apply_fog_factor(sec->floorpicnum, sec->floorshade, sec->visibility, sec->floorpal, 0.005f);

			//Use clamping for tiled sky textures
			for (int i = (1 << dapskybits) - 1; i>0; i--)
				if (dapskyoff[i] != dapskyoff[i - 1])
				{
					break;
				}

			if (!usehightile || !hicfindskybox(globalpicnum, globalpal))
			{
				float const dd = fxdimen*.0000001f; //Adjust sky depth based on screen size!
				float vv[2];
				float t = (float)((1 << (picsiz[globalpicnum] & 15)) << dapskybits);
				vv[1] = dd*((float)xdimscale*fviewingrange) * (1.f / (65536.f*65536.f));
				vv[0] = dd*((float)((tilesiz[globalpicnum].y >> 1) + parallaxyoffs_override/*+g_psky.yoffs*/)) - vv[1] * ghoriz;
				int i = (1 << (picsiz[globalpicnum] >> 4)); if (i != tilesiz[globalpicnum].y) i += i;
				vec3f_t o;

				//Hack to draw black rectangle below sky when looking down...
				xtex.d = xtex.u = xtex.v = 0;

				ytex.d = gxyaspect * (1.f / 262144.f);
				ytex.u = 0;
				ytex.v = (float)(tilesiz[globalpicnum].y - 1) * ytex.d;

				otex.d = -ghoriz * ytex.d;
				otex.u = 0;
				otex.v = (float)(tilesiz[globalpicnum].y - 1) * otex.d;

				o.y = ((float)tilesiz[globalpicnum].y*dd - vv[0]) / vv[1];

				if ((o.y > fy0) && (o.y > fy1))
					domost(x0, o.y, x1, o.y, sectnum);
				else if ((o.y > fy0) != (o.y > fy1))
				{
					//  fy0                      fy1
					//     \                    /
					//oy----------      oy----------
					//        \              /
					//         fy1        fy0
					o.x = (o.y - fy0)*(x1 - x0) / (fy1 - fy0) + x0;
					if (o.y > fy0)
					{
						domost(x0, o.y, o.x, o.y, sectnum);
						domost(o.x, o.y, x1, fy1, sectnum);
					}
					else
					{
						domost(x0, fy0, o.x, o.y, sectnum);
						domost(o.x, o.y, x1, o.y, sectnum);
					}
				}
				else
					domost(x0, fy0, x1, fy1, sectnum);


				xtex.d = xtex.v = 0;
				ytex.d = ytex.u = 0;
				otex.d = dd;
				xtex.u = otex.d * (t * (float)((uint64_t)(xdimscale * yxaspect) * viewingrange)) *
					(1.f / (16384.0 * 65536.0 * 65536.0 * 5.0 * 1024.0));
				ytex.v = vv[1];
				otex.v = r_parallaxskypanning ? vv[0] + dd*(float)sec->floorypanning*(float)i*(1.f / 256.f) : vv[0];

				i = globalpicnum;
				float const r = (fy1 - fy0) / (x1 - x0); //slope of line
				o.y = fviewingrange / (ghalfx*256.f); o.z = 1.f / o.y;

				int y = ((((int32_t)((x0 - ghalfx)*o.y)) + globalang) >> (11 - dapskybits));
				float fx = x0;
				do
				{
					globalpicnum = dapskyoff[y&((1 << dapskybits) - 1)] + i;
					otex.u = otex.d*(t*((float)(globalang - (y << (11 - dapskybits)))) * (1.f / 2048.f) + (float)((r_parallaxskypanning) ? sec->floorxpanning : 0)) - xtex.u*ghalfx;
					y++;
					o.x = fx; fx = ((float)((y << (11 - dapskybits)) - globalang))*o.z + ghalfx;
					if (fx > x1) { fx = x1; i = -1; }

					pow2xsplit = 0; domost(o.x, (o.x - x0)*r + fy0, fx, (fx - x0)*r + fy0, sectnum); //flor
				} while (i >= 0);
			}
			else  //NOTE: code copied from ceiling code... lots of duplicated stuff :/
			{
				//Skybox code for parallax floor!
				float sky_t0, sky_t1; // _nx0, _ny0, _nx1, _ny1;
				float sky_ryp0, sky_ryp1, sky_x0, sky_x1, sky_cy0, sky_fy0, sky_cy1, sky_fy1, sky_ox0, sky_ox1;
				static vec2f_t const skywal[4] = { { -512, -512 },{ 512, -512 },{ 512, 512 },{ -512, 512 } };

				pow2xsplit = 0;
				//skyclamphack = 1;

				for (int i = 0; i<4; i++)
				{
					walpos = skywal[i & 3];
					vec2f_t skyp0 = { walpos.y * gcosang - walpos.x * gsinang,
						walpos.x * gcosang2 + walpos.y * gsinang2 };

					walpos = skywal[(i + 1) & 3];
					vec2f_t skyp1 = { walpos.y * gcosang - walpos.x * gsinang,
						walpos.x * gcosang2 + walpos.y * gsinang2 };

					vec2f_t const oskyp0 = skyp0;

					//Clip to close parallel-screen plane
					if (skyp0.y < SCISDIST)
					{
						if (skyp1.y < SCISDIST) continue;
						sky_t0 = (SCISDIST - skyp0.y) / (skyp1.y - skyp0.y); skyp0.x = (skyp1.x - skyp0.x)*sky_t0 + skyp0.x; skyp0.y = SCISDIST;
					}
					else { sky_t0 = 0.f; }

					if (skyp1.y < SCISDIST)
					{
						sky_t1 = (SCISDIST - oskyp0.y) / (skyp1.y - oskyp0.y); skyp1.x = (skyp1.x - oskyp0.x)*sky_t1 + oskyp0.x; skyp1.y = SCISDIST;
					}
					else { sky_t1 = 1.f; }

					sky_ryp0 = 1.f / skyp0.y; sky_ryp1 = 1.f / skyp1.y;

					//Generate screen coordinates for front side of wall
					sky_x0 = ghalfx*skyp0.x*sky_ryp0 + ghalfx;
					sky_x1 = ghalfx*skyp1.x*sky_ryp1 + ghalfx;
					if ((sky_x1 <= sky_x0) || (sky_x0 >= x1) || (x0 >= sky_x1)) continue;

					sky_ryp0 *= gyxscale; sky_ryp1 *= gyxscale;

					sky_cy0 = -8192.f*sky_ryp0 + ghoriz;
					sky_fy0 = 8192.f*sky_ryp0 + ghoriz;
					sky_cy1 = -8192.f*sky_ryp1 + ghoriz;
					sky_fy1 = 8192.f*sky_ryp1 + ghoriz;

					sky_ox0 = sky_x0; sky_ox1 = sky_x1;

					//Make sure: x0<=_x0<_x1<=x1
					float nfy[2] = { fy0, fy1 };

					if (sky_x0 < x0)
					{
						float const t = (x0 - sky_x0) / (sky_x1 - sky_x0);
						sky_cy0 += (sky_cy1 - sky_cy0)*t;
						sky_fy0 += (sky_fy1 - sky_fy0)*t;
						sky_x0 = x0;
					}
					else if (sky_x0 > x0) nfy[0] += (sky_x0 - x0)*(fy1 - fy0) / (x1 - x0);

					if (sky_x1 > x1)
					{
						float const t = (x1 - sky_x1) / (sky_x1 - sky_x0);
						sky_cy1 += (sky_cy1 - sky_cy0)*t;
						sky_fy1 += (sky_fy1 - sky_fy0)*t;
						sky_x1 = x1;
					}
					else if (sky_x1 < x1) nfy[1] += (sky_x1 - x1)*(fy1 - fy0) / (x1 - x0);

					//   (skybox floor)
					//(_x0,_fy0)-(_x1,_fy1)
					//   (skybox wall)
					//(_x0,_cy0)-(_x1,_cy1)
					//   (skybox ceiling)
					//(_x0,nfy0)-(_x1,nfy1)

					//floor of skybox
					drawingskybox = 6; //floor/6th texture/index 5 of skybox
					float const ft[4] = { 512 / 16, 512 / -16, fcosglobalang * (1.f / 2147483648.f),
						fsinglobalang * (1.f / 2147483648.f) };

					xtex.d = 0;
					ytex.d = gxyaspect*(1.f / 4194304.f);
					otex.d = -ghoriz*ytex.d;
					xtex.u = ft[3] * fviewingrange*(-1.0 / 65536.0);
					xtex.v = ft[2] * fviewingrange*(-1.0 / 65536.0);
					ytex.u = ft[0] * ytex.d; ytex.v = ft[1] * ytex.d;
					otex.u = ft[0] * otex.d; otex.v = ft[1] * otex.d;
					otex.u += (ft[2] - xtex.u)*ghalfx;
					otex.v -= (ft[3] + xtex.v)*ghalfx;
					xtex.v = -xtex.v; ytex.v = -ytex.v; otex.v = -otex.v; //y-flip skybox floor

					if ((sky_fy0 > nfy[0]) && (sky_fy1 > nfy[1]))
						domost(sky_x0, sky_fy0, sky_x1, sky_fy1, sectnum);
					else if ((sky_fy0 > nfy[0]) != (sky_fy1 > nfy[1]))
					{
						//(ox,oy) is intersection of: (_x0,_fy0)-(_x1,_fy1)
						//                            (_x0,nfy0)-(_x1,nfy1)
						float const t = (sky_fy0 - nfy[0]) / (nfy[1] - nfy[0] - sky_fy1 + sky_fy0);
						vec2f_t const o = { sky_x0 + (sky_x1 - sky_x0)*t, sky_fy0 + (sky_fy1 - sky_fy0)*t };
						if (nfy[0] > sky_fy0)
						{
							domost(sky_x0, nfy[0], o.x, o.y, sectnum);
							domost(o.x, o.y, sky_x1, sky_fy1, sectnum);
						}
						else
						{
							domost(sky_x0, sky_fy0, o.x, o.y, sectnum);
							domost(o.x, o.y, sky_x1, nfy[1], sectnum);
						}
					}
					else
						domost(sky_x0, nfy[0], sky_x1, nfy[1], sectnum);

					//wall of skybox
					drawingskybox = i + 1; //i+1th texture/index i of skybox
					xtex.d = (sky_ryp0 - sky_ryp1)*gxyaspect*(1.f / 512.f) / (sky_ox0 - sky_ox1);
					ytex.d = 0;
					otex.d = sky_ryp0*gxyaspect*(1.f / 512.f) - xtex.d*sky_ox0;
					xtex.u = (sky_t0*sky_ryp0 - sky_t1*sky_ryp1)*gxyaspect*(64.f / 512.f) / (sky_ox0 - sky_ox1);
					otex.u = sky_t0*sky_ryp0*gxyaspect*(64.f / 512.f) - xtex.u*sky_ox0;
					ytex.u = 0;
					sky_t0 = -8192.f*sky_ryp0 + ghoriz;
					sky_t1 = -8192.f*sky_ryp1 + ghoriz;
					float const t = ((xtex.d*sky_ox0 + otex.d)*8.f) / ((sky_ox1 - sky_ox0) * sky_ryp0 * 2048.f);
					xtex.v = (sky_t0 - sky_t1)*t;
					ytex.v = (sky_ox1 - sky_ox0)*t;
					otex.v = -xtex.v*sky_ox0 - ytex.v*sky_t0;

					if ((sky_cy0 > nfy[0]) && (sky_cy1 > nfy[1]))
						domost(sky_x0, sky_cy0, sky_x1, sky_cy1, sectnum);
					else if ((sky_cy0 > nfy[0]) != (sky_cy1 > nfy[1]))
					{
						//(ox,oy) is intersection of: (_x0,_fy0)-(_x1,_fy1)
						//                            (_x0,nfy0)-(_x1,nfy1)
						float const t = (sky_cy0 - nfy[0]) / (nfy[1] - nfy[0] - sky_cy1 + sky_cy0);
						vec2f_t const o = { sky_x0 + (sky_x1 - sky_x0) * t, sky_cy0 + (sky_cy1 - sky_cy0) * t };
						if (nfy[0] > sky_cy0)
						{
							domost(sky_x0, nfy[0], o.x, o.y, sectnum);
							domost(o.x, o.y, sky_x1, sky_cy1, sectnum);
						}
						else
						{
							domost(sky_x0, sky_cy0, o.x, o.y, sectnum);
							domost(o.x, o.y, sky_x1, nfy[1], sectnum);
						}
					}
					else
						domost(sky_x0, nfy[0], sky_x1, nfy[1], sectnum);
				}

				//Ceiling of skybox
				drawingskybox = 5; //ceiling/5th texture/index 4 of skybox
				float const ft[4] = { 512 / 16, -512 / -16, fcosglobalang * (1.f / 2147483648.f),
					fsinglobalang * (1.f / 2147483648.f) };

				xtex.d = 0;
				ytex.d = gxyaspect*(-1.f / 4194304.f);
				otex.d = -ghoriz*ytex.d;
				xtex.u = ft[3] * fviewingrange*(-1.0 / 65536.0);
				xtex.v = ft[2] * fviewingrange*(-1.0 / 65536.0);
				ytex.u = ft[0] * ytex.d; ytex.v = ft[1] * ytex.d;
				otex.u = ft[0] * otex.d; otex.v = ft[1] * otex.d;
				otex.u += (ft[2] - xtex.u)*ghalfx;
				otex.v -= (ft[3] + xtex.v)*ghalfx;

				domost(x0, fy0, x1, fy1, sectnum);

				//skyclamphack = 0;
				drawingskybox = 0;
			}

			//skyclamphack = 0;
			//if (!nofog)
			//	bglEnable(GL_FOG);
		}

		// Ceiling

		globalpicnum = sec->ceilingpicnum;
		globalshade = sec->ceilingshade;
		globalpal = sec->ceilingpal;
		globalorientation = sec->ceilingstat;
		globvis = (sector[sectnum].visibility != 0) ?
			mulscale4(globalcisibility, (uint8_t)(sector[sectnum].visibility + 16)) :
			globalcisibility;

		DO_TILE_ANIM(globalpicnum, sectnum);


		dapskyoff = getpsky(globalpicnum, NULL, &dapskybits);

		global_cf_fogpal = sec->fogpal;
		global_cf_shade = sec->ceilingshade, global_cf_pal = sec->ceilingpal; global_cf_z = sec->ceilingz;  // REFACT
		global_cf_xpanning = sec->ceilingxpanning; global_cf_ypanning = sec->ceilingypanning, global_cf_heinum = sec->ceilingheinum;
		global_getzofslope_func = &getceilzofslope;

		if (!(globalorientation & 1))
		{
#ifdef YAX_ENABLE
			if (globalposz >= sec->ceilingz || yax_getbunch(sectnum, YAX_CEILING) < 0 || yax_getnextwall(wallnum, YAX_CEILING) >= 0)
#endif
				internal_nonparallaxed(n0, n1, ryp0, ryp1, x0, x1, cy0, cy1, sectnum);
		}
		else if ((nextsectnum < 0) || (!(sector[nextsectnum].ceilingstat & 1)))
		{
			//Parallaxing sky... hacked for Ken's mountain texture
		//	if (!nofog) calc_and_apply_fog_factor(sec->ceilingpicnum, sec->ceilingshade, sec->visibility, sec->ceilingpal, 0.005f);

			//Use clamping for tiled sky textures
			for (int i = (1 << dapskybits) - 1; i>0; i--)
				if (dapskyoff[i] != dapskyoff[i - 1])
				{
					break;
				}

			if (!usehightile || !hicfindskybox(globalpicnum, globalpal))
			{
				float const dd = fxdimen*.0000001f; //Adjust sky depth based on screen size!
				float vv[2];
				float t = (float)((1 << (picsiz[globalpicnum] & 15)) << dapskybits);
				vv[1] = dd*((float)xdimscale*fviewingrange) * (1.f / (65536.f*65536.f));
				vv[0] = dd*((float)((tilesiz[globalpicnum].y >> 1) + parallaxyoffs_override/*+g_psky.yoffs*/)) - vv[1] * ghoriz;
				int i = (1 << (picsiz[globalpicnum] >> 4)); if (i != tilesiz[globalpicnum].y) i += i;
				vec3f_t o;

				//Hack to draw color rectangle above sky when looking up...
				xtex.d = xtex.u = xtex.v = 0;

				ytex.d = gxyaspect * (1.f / -262144.f);
				ytex.u = 0;
				ytex.v = 0;

				otex.d = -ghoriz * ytex.d;
				otex.u = 0;
				otex.v = 0;

				o.y = -vv[0] / vv[1];

				if ((o.y < cy0) && (o.y < cy1))
					domost(x1, o.y, x0, o.y, sectnum);
				else if ((o.y < cy0) != (o.y < cy1))
				{
					/*         cy1        cy0
					//        /              \
					//oy----------      oy---------
					//    /                   \
					//  cy0                     cy1 */
					o.x = (o.y - cy0)*(x1 - x0) / (cy1 - cy0) + x0;
					if (o.y < cy0)
					{
						domost(o.x, o.y, x0, o.y, sectnum);
						domost(x1, cy1, o.x, o.y, sectnum);
					}
					else
					{
						domost(o.x, o.y, x0, cy0, sectnum);
						domost(x1, o.y, o.x, o.y, sectnum);
					}
				}
				else
					domost(x1, cy1, x0, cy0, sectnum);

				xtex.d = xtex.v = 0;
				ytex.d = ytex.u = 0;
				otex.d = dd;
				xtex.u = otex.d * (t * (float)((uint64_t)(xdimscale * yxaspect) * viewingrange)) *
					(1.f / (16384.0 * 65536.0 * 65536.0 * 5.0 * 1024.0));
				ytex.v = vv[1];
				otex.v = r_parallaxskypanning ? vv[0] + dd*(float)sec->ceilingypanning*(float)i*(1.f / 256.f) : vv[0];

				i = globalpicnum;
				float const r = (cy1 - cy0) / (x1 - x0); //slope of line
				o.y = fviewingrange / (ghalfx*256.f); o.z = 1.f / o.y;

				int y = ((((int32_t)((x0 - ghalfx)*o.y)) + globalang) >> (11 - dapskybits));
				float fx = x0;
				do
				{
					globalpicnum = dapskyoff[y&((1 << dapskybits) - 1)] + i;
					otex.u = otex.d*(t*((float)(globalang - (y << (11 - dapskybits)))) * (1.f / 2048.f) + (float)((r_parallaxskypanning) ? sec->ceilingxpanning : 0)) - xtex.u*ghalfx;
					y++;
					o.x = fx; fx = ((float)((y << (11 - dapskybits)) - globalang))*o.z + ghalfx;
					if (fx > x1) { fx = x1; i = -1; }

					pow2xsplit = 0; domost(fx, (fx - x0)*r + cy0, o.x, (o.x - x0)*r + cy0, sectnum); //ceil
				} while (i >= 0);
			}
			else
			{
				//Skybox code for parallax ceiling!
				float sky_t0, sky_t1; // _nx0, _ny0, _nx1, _ny1;
				float sky_ryp0, sky_ryp1, sky_x0, sky_x1, sky_cy0, sky_fy0, sky_cy1, sky_fy1, sky_ox0, sky_ox1;
				static vec2f_t const skywal[4] = { { -512, -512 },{ 512, -512 },{ 512, 512 },{ -512, 512 } };

				pow2xsplit = 0;
			//	skyclamphack = 1;

				for (int i = 0; i<4; i++)
				{
					walpos = skywal[i & 3];
					vec2f_t skyp0 = { walpos.y * gcosang - walpos.x * gsinang,
						walpos.x * gcosang2 + walpos.y * gsinang2 };

					walpos = skywal[(i + 1) & 3];
					vec2f_t skyp1 = { walpos.y * gcosang - walpos.x * gsinang,
						walpos.x * gcosang2 + walpos.y * gsinang2 };

					vec2f_t const oskyp0 = skyp0;

					//Clip to close parallel-screen plane
					if (skyp0.y < SCISDIST)
					{
						if (skyp1.y < SCISDIST) continue;
						sky_t0 = (SCISDIST - skyp0.y) / (skyp1.y - skyp0.y); skyp0.x = (skyp1.x - skyp0.x)*sky_t0 + skyp0.x; skyp0.y = SCISDIST;
					}
					else { sky_t0 = 0.f; }

					if (skyp1.y < SCISDIST)
					{
						sky_t1 = (SCISDIST - oskyp0.y) / (skyp1.y - oskyp0.y); skyp1.x = (skyp1.x - oskyp0.x)*sky_t1 + oskyp0.x; skyp1.y = SCISDIST;
					}
					else { sky_t1 = 1.f; }

					sky_ryp0 = 1.f / skyp0.y; sky_ryp1 = 1.f / skyp1.y;

					//Generate screen coordinates for front side of wall
					sky_x0 = ghalfx*skyp0.x*sky_ryp0 + ghalfx;
					sky_x1 = ghalfx*skyp1.x*sky_ryp1 + ghalfx;
					if ((sky_x1 <= sky_x0) || (sky_x0 >= x1) || (x0 >= sky_x1)) continue;

					sky_ryp0 *= gyxscale; sky_ryp1 *= gyxscale;

					sky_cy0 = -8192.f*sky_ryp0 + ghoriz;
					sky_fy0 = 8192.f*sky_ryp0 + ghoriz;
					sky_cy1 = -8192.f*sky_ryp1 + ghoriz;
					sky_fy1 = 8192.f*sky_ryp1 + ghoriz;

					sky_ox0 = sky_x0; sky_ox1 = sky_x1;

					//Make sure: x0<=_x0<_x1<=x1
					float ncy[2] = { cy0, cy1 };

					if (sky_x0 < x0)
					{
						float const t = (x0 - sky_x0) / (sky_x1 - sky_x0);
						sky_cy0 += (sky_cy1 - sky_cy0)*t;
						sky_fy0 += (sky_fy1 - sky_fy0)*t;
						sky_x0 = x0;
					}
					else if (sky_x0 > x0) ncy[0] += (sky_x0 - x0)*(cy1 - cy0) / (x1 - x0);

					if (sky_x1 > x1)
					{
						float const t = (x1 - sky_x1) / (sky_x1 - sky_x0);
						sky_cy1 += (sky_cy1 - sky_cy0)*t;
						sky_fy1 += (sky_fy1 - sky_fy0)*t;
						sky_x1 = x1;
					}
					else if (sky_x1 < x1) ncy[1] += (sky_x1 - x1)*(cy1 - cy0) / (x1 - x0);

					//   (skybox ceiling)
					//(_x0,_cy0)-(_x1,_cy1)
					//   (skybox wall)
					//(_x0,_fy0)-(_x1,_fy1)
					//   (skybox floor)
					//(_x0,ncy0)-(_x1,ncy1)

					//ceiling of skybox
					drawingskybox = 5; //ceiling/5th texture/index 4 of skybox
					float const ft[4] = { 512 / 16, -512 / -16, fcosglobalang * (1.f / 2147483648.f),
						fsinglobalang * (1.f / 2147483648.f) };

					xtex.d = 0;
					ytex.d = gxyaspect*(-1.f / 4194304.f);
					otex.d = -ghoriz*ytex.d;
					xtex.u = ft[3] * fviewingrange*(-1.0 / 65536.0);
					xtex.v = ft[2] * fviewingrange*(-1.0 / 65536.0);
					ytex.u = ft[0] * ytex.d; ytex.v = ft[1] * ytex.d;
					otex.u = ft[0] * otex.d; otex.v = ft[1] * otex.d;
					otex.u += (ft[2] - xtex.u)*ghalfx;
					otex.v -= (ft[3] + xtex.v)*ghalfx;


					if ((sky_cy0 < ncy[0]) && (sky_cy1 < ncy[1]))
						domost(sky_x1, sky_cy1, sky_x0, sky_cy0, sectnum);
					else if ((sky_cy0 < ncy[0]) != (sky_cy1 < ncy[1]))
					{
						//(ox,oy) is intersection of: (_x0,_cy0)-(_x1,_cy1)
						//                            (_x0,ncy0)-(_x1,ncy1)
						float const t = (sky_cy0 - ncy[0]) / (ncy[1] - ncy[0] - sky_cy1 + sky_cy0);
						vec2f_t const o = { sky_x0 + (sky_x1 - sky_x0)*t, sky_cy0 + (sky_cy1 - sky_cy0)*t };
						if (ncy[0] < sky_cy0)
						{
							domost(o.x, o.y, sky_x0, ncy[0], sectnum);
							domost(sky_x1, sky_cy1, o.x, o.y, sectnum);
						}
						else
						{
							domost(o.x, o.y, sky_x0, sky_cy0, sectnum);
							domost(sky_x1, ncy[1], o.x, o.y, sectnum);
						}
					}
					else
						domost(sky_x1, ncy[1], sky_x0, ncy[0], sectnum);

					//wall of skybox
					drawingskybox = i + 1; //i+1th texture/index i of skybox
					xtex.d = (sky_ryp0 - sky_ryp1)*gxyaspect*(1.f / 512.f) / (sky_ox0 - sky_ox1);
					ytex.d = 0;
					otex.d = sky_ryp0*gxyaspect*(1.f / 512.f) - xtex.d*sky_ox0;
					xtex.u = (sky_t0*sky_ryp0 - sky_t1*sky_ryp1)*gxyaspect*(64.f / 512.f) / (sky_ox0 - sky_ox1);
					otex.u = sky_t0*sky_ryp0*gxyaspect*(64.f / 512.f) - xtex.u*sky_ox0;
					ytex.u = 0;
					sky_t0 = -8192.f*sky_ryp0 + ghoriz;
					sky_t1 = -8192.f*sky_ryp1 + ghoriz;
					float const t = ((xtex.d*sky_ox0 + otex.d)*8.f) / ((sky_ox1 - sky_ox0) * sky_ryp0 * 2048.f);
					xtex.v = (sky_t0 - sky_t1)*t;
					ytex.v = (sky_ox1 - sky_ox0)*t;
					otex.v = -xtex.v*sky_ox0 - ytex.v*sky_t0;

					if ((sky_fy0 < ncy[0]) && (sky_fy1 < ncy[1]))
						domost(sky_x1, sky_fy1, sky_x0, sky_fy0, sectnum);
					else if ((sky_fy0 < ncy[0]) != (sky_fy1 < ncy[1]))
					{
						//(ox,oy) is intersection of: (_x0,_fy0)-(_x1,_fy1)
						//                            (_x0,ncy0)-(_x1,ncy1)
						float const t = (sky_fy0 - ncy[0]) / (ncy[1] - ncy[0] - sky_fy1 + sky_fy0);
						vec2f_t const o = { sky_x0 + (sky_x1 - sky_x0) * t, sky_fy0 + (sky_fy1 - sky_fy0) * t };
						if (ncy[0] < sky_fy0)
						{
							domost(o.x, o.y, sky_x0, ncy[0], sectnum);
							domost(sky_x1, sky_fy1, o.x, o.y, sectnum);
						}
						else
						{
							domost(o.x, o.y, sky_x0, sky_fy0, sectnum);
							domost(sky_x1, ncy[1], o.x, o.y, sectnum);
						}
					}
					else
						domost(sky_x1, ncy[1], sky_x0, ncy[0], sectnum);
				}

				//Floor of skybox
				drawingskybox = 6; //floor/6th texture/index 5 of skybox
				float const ft[4] = { 512 / 16, 512 / -16, fcosglobalang * (1.f / 2147483648.f),
					fsinglobalang * (1.f / 2147483648.f) };

				xtex.d = 0;
				ytex.d = gxyaspect*(1.f / 4194304.f);
				otex.d = -ghoriz*ytex.d;
				xtex.u = ft[3] * fviewingrange*(-1.0 / 65536.0);
				xtex.v = ft[2] * fviewingrange*(-1.0 / 65536.0);
				ytex.u = ft[0] * ytex.d; ytex.v = ft[1] * ytex.d;
				otex.u = ft[0] * otex.d; otex.v = ft[1] * otex.d;
				otex.u += (ft[2] - xtex.u)*ghalfx;
				otex.v -= (ft[3] + xtex.v)*ghalfx;
				xtex.v = -xtex.v; ytex.v = -ytex.v; otex.v = -otex.v; //y-flip skybox floor
				domost(x1, cy1, x0, cy0, sectnum);

				//skyclamphack = 0;
				drawingskybox = 0;
			}

			//skyclamphack = 0;
			//if (!nofog)
			//	bglEnable(GL_FOG);
		}

		// Wall

		xtex.d = (ryp0 - ryp1)*gxyaspect / (x0 - x1);
		ytex.d = 0;
		otex.d = ryp0*gxyaspect - xtex.d*x0;

		xtex.u = (t0*ryp0 - t1*ryp1)*gxyaspect*(float)wal->xrepeat*8.f / (x0 - x1);
		otex.u = t0*ryp0*gxyaspect*(float)wal->xrepeat*8.f - xtex.u*x0;
		otex.u += (float)wal->xpanning*otex.d;
		xtex.u += (float)wal->xpanning*xtex.d;
		ytex.u = 0;

		float const ogux = xtex.u, oguy = ytex.u, oguo = otex.u;

		Bassert(domostpolymethod == 0);
		domostpolymethod = DAMETH_WALL;

		if (nextsectnum >= 0)
		{
			getzsofslope(nextsectnum,/*Blrintf(nx0)*/(int)n0.x,/*Blrintf(ny0)*/(int)n0.y, &cz, &fz);
			float const ocy0 = ((float)(cz - globalposz))*ryp0 + ghoriz;
			float const ofy0 = ((float)(fz - globalposz))*ryp0 + ghoriz;
			getzsofslope(nextsectnum,/*Blrintf(nx1)*/(int)n1.x,/*Blrintf(ny1)*/(int)n1.y, &cz, &fz);
			float const ocy1 = ((float)(cz - globalposz))*ryp1 + ghoriz;
			float const ofy1 = ((float)(fz - globalposz))*ryp1 + ghoriz;

			if ((wal->cstat & 48) == 16) maskwall[maskwallcnt++] = z;

			if (((cy0 < ocy0) || (cy1 < ocy1)) && (!((sec->ceilingstat&sector[nextsectnum].ceilingstat) & 1)))
			{
				globalpicnum = wal->picnum; globalshade = wal->shade; globalpal = (int32_t)((uint8_t)wal->pal);
				globvis = globalvisibility;
				if (sector[sectnum].visibility != 0) globvis = mulscale4(globvis, (uint8_t)(sector[sectnum].visibility + 16));

				DO_TILE_ANIM(globalpicnum, wallnum + 16384);

				int i = (!(wal->cstat & 4)) ? sector[nextsectnum].ceilingz : sec->ceilingz;

				// over
				calc_ypanning(i, ryp0, ryp1, x0, x1, wal->ypanning, wal->yrepeat, wal->cstat & 4);

				if (wal->cstat & 8) //xflip
				{
					float const t = (float)(wal->xrepeat * 8 + wal->xpanning * 2);
					xtex.u = xtex.d*t - xtex.u;
					ytex.u = ytex.d*t - ytex.u;
					otex.u = otex.d*t - otex.u;
				}
				if (wal->cstat & 256) { xtex.v = -xtex.v; ytex.v = -ytex.v; otex.v = -otex.v; } //yflip

			//	if (!nofog) calc_and_apply_fog(wal->picnum, fogpal_shade(sec, wal->shade), sec->visibility, get_floor_fogpal(sec));

				pow2xsplit = 1; domost(x1, ocy1, x0, ocy0, sectnum);
				if (wal->cstat & 8) { xtex.u = ogux; ytex.u = oguy; otex.u = oguo; }
			}
			if (((ofy0 < fy0) || (ofy1 < fy1)) && (!((sec->floorstat&sector[nextsectnum].floorstat) & 1)))
			{
				twalltype *nwal;

				if (!(wal->cstat & 2)) nwal = wal;
				else
				{
					nwal = (twalltype *)&wall[wal->nextwall];
					otex.u += (float)(nwal->xpanning - wal->xpanning) * otex.d;
					xtex.u += (float)(nwal->xpanning - wal->xpanning) * xtex.d;
					ytex.u += (float)(nwal->xpanning - wal->xpanning) * ytex.d;
				}
				globalpicnum = nwal->picnum; globalshade = nwal->shade; globalpal = (int32_t)((uint8_t)nwal->pal);
				globvis = globalvisibility;
				if (sector[sectnum].visibility != 0) globvis = mulscale4(globvis, (uint8_t)(sector[sectnum].visibility + 16));

				DO_TILE_ANIM(globalpicnum, wallnum + 16384);

				int i = (!(nwal->cstat & 4)) ? sector[nextsectnum].floorz : sec->ceilingz;

				// under
				calc_ypanning(i, ryp0, ryp1, x0, x1, nwal->ypanning, wal->yrepeat, !(nwal->cstat & 4));

				if (wal->cstat & 8) //xflip
				{
					float const t = (float)(wal->xrepeat * 8 + nwal->xpanning * 2);
					xtex.u = xtex.d*t - xtex.u;
					ytex.u = ytex.d*t - ytex.u;
					otex.u = otex.d*t - otex.u;
				}
				if (nwal->cstat & 256) { xtex.v = -xtex.v; ytex.v = -ytex.v; otex.v = -otex.v; } //yflip

				//if (!nofog) calc_and_apply_fog(nwal->picnum, fogpal_shade(sec, nwal->shade), sec->visibility, get_floor_fogpal(sec));

				pow2xsplit = 1; domost(x0, ofy0, x1, ofy1, sectnum);
				if (wal->cstat&(2 + 8)) { otex.u = oguo; xtex.u = ogux; ytex.u = oguy; }
			}
		}

		if ((nextsectnum < 0) || (wal->cstat & 32))   //White/1-way wall
		{
			do
			{
				const int maskingOneWay = (nextsectnum >= 0 && (wal->cstat & 32));

				if (maskingOneWay)
				{
					vec2_t n, pos = { globalposx, globalposy };
					if (!getclosestpointonwall(&pos, wallnum, &n) && klabs(pos.x - n.x) + klabs(pos.y - n.y) <= 128)
						break;
				}

				globalpicnum = (nextsectnum < 0) ? wal->picnum : wal->overpicnum;

				globalshade = wal->shade;
				globalpal = wal->pal;
				globvis = (sector[sectnum].visibility != 0) ?
					mulscale4(globalvisibility, (uint8_t)(sector[sectnum].visibility + 16)) :
					globalvisibility;

				DO_TILE_ANIM(globalpicnum, wallnum + 16384);

				int i;
				int const nwcs4 = !(wal->cstat & 4);

				if (nextsectnum >= 0) { i = nwcs4 ? nextsec->ceilingz : sec->ceilingz; }
				else { i = nwcs4 ? sec->ceilingz : sec->floorz; }

				// white / 1-way
				calc_ypanning(i, ryp0, ryp1, x0, x1, wal->ypanning, wal->yrepeat, nwcs4 && !maskingOneWay);

				if (wal->cstat & 8) //xflip
				{
					float const t = (float)(wal->xrepeat * 8 + wal->xpanning * 2);
					xtex.u = xtex.d*t - xtex.u;
					ytex.u = ytex.d*t - ytex.u;
					otex.u = otex.d*t - otex.u;
				}
				if (wal->cstat & 256) { xtex.v = -xtex.v; ytex.v = -ytex.v; otex.v = -otex.v; } //yflip

				//if (!nofog) calc_and_apply_fog(wal->picnum, fogpal_shade(sec, wal->shade), sec->visibility, get_floor_fogpal(sec));
				pow2xsplit = 1; domost(x0, cy0, x1, cy1, sectnum);
			} while (0);
		}

		domostpolymethod = 0;

		if (nextsectnum >= 0)
			if ((!(gotsector[nextsectnum >> 3] & pow2char[nextsectnum & 7])) && testvisiblemost(x0, x1))
				scansector(nextsectnum);
	}
}

int32_t Build3DBoardPolymost::bunchfront(const int32_t b1, const int32_t b2)
{
	int b1f = bunchfirst[b1];
	const float x2b2 = dxb2[bunchlast[b2]];
	const float x1b1 = dxb1[b1f];

	if (x1b1 >= x2b2)
		return -1;

	int b2f = bunchfirst[b2];
	const float x1b2 = dxb1[b2f];

	if (x1b2 >= dxb2[bunchlast[b1]])
		return -1;

	if (x1b1 >= x1b2)
	{
		for (; dxb2[b2f] <= x1b1; b2f = bunchp2[b2f]);
		return wallfront(b1f, b2f);
	}

	for (; dxb2[b1f] <= x1b2; b1f = bunchp2[b1f]);
	return wallfront(b1f, b2f);
}

void Build3DBoardPolymost::scansector(int32_t sectnum)
{
	if (sectnum < 0) return;

	sectorborder[0] = sectnum;
	int sectorbordercnt = 1;
	do
	{
		sectnum = sectorborder[--sectorbordercnt];

		//for (int z = headspritesect[sectnum]; z >= 0; z = nextspritesect[z])
		//{
		//	tspritetype const * const spr = (tspritetype *)&sprite[z];
		//	if ((((spr->cstat & 0x8000) == 0) || (showinvisibility)) &&
		//		(spr->xrepeat > 0) && (spr->yrepeat > 0))
		//	{
		//		vec2_t const s = { spr->x - globalposx, spr->y - globalposy };
		//
		//		if ((spr->cstat & 48) || (usemodels && tile2model[spr->picnum].modelid >= 0) || ((s.x * gcosang) + (s.y * gsinang) > 0))
		//		{
		//			if ((spr->cstat&(64 + 48)) != (64 + 16) || dmulscale6(sintable[(spr->ang + 512) & 2047], -s.x, sintable[spr->ang & 2047], -s.y) > 0)
		//				if (engine_addtsprite(z, sectnum))
		//					break;
		//		}
		//	}
		//}

		gotsector[sectnum >> 3] |= pow2char[sectnum & 7];

		int const bunchfrst = numbunches;
		int const numscansbefore = numscans;

		int const startwall = sector[sectnum].wallptr;
		int const endwall = sector[sectnum].wallnum + startwall;
		int scanfirst = numscans;
		vec2f_t p2 = { 0, 0 };

		twalltype *wal;
		int z;

		AddUniqueSector(sectnum);

		for (z = startwall, wal = (twalltype *)&wall[z]; z<endwall; z++, wal++)
		{
			twalltype const * const wal2 = (twalltype *)&wall[wal->point2];
			vec2f_t const fp1 = { (float)(wal->x - globalposx), (float)(wal->y - globalposy) };
			vec2f_t const fp2 = { (float)(wal2->x - globalposx), (float)(wal2->y - globalposy) };

			int const nextsectnum = wal->nextsector; //Scan close sectors

			vec2f_t p1;

			AddUniqueSector(nextsectnum);

			if (nextsectnum >= 0 /*&& !(wal->cstat&32)*/ && sectorbordercnt < ARRAY_SSIZE(sectorborder))
			{
#ifdef YAX_ENABLE
				if (yax_nomaskpass == 0 || !yax_isislandwall(z, !yax_globalcf) || (yax_nomaskdidit = 1, 0))
#endif
					if ((gotsector[nextsectnum >> 3] & pow2char[nextsectnum & 7]) == 0)
					{
						float const d = fp1.x*fp2.y - fp2.x*fp1.y;
						p1.x = fp2.x - fp1.x;
						p1.y = fp2.y - fp1.y;

						if (d*d <= (p1.x*p1.x + p1.y*p1.y) * (SCISDIST*SCISDIST*260.f))
						{
							sectorborder[sectorbordercnt++] = nextsectnum;
							gotsector[nextsectnum >> 3] |= pow2char[nextsectnum & 7];
						}
					}
			}

			if ((z == startwall) || (wall[z - 1].point2 != z))
			{
				p1.x = ((fp1.y * fcosglobalang) - (fp1.x * fsinglobalang)) * (1.0f / 64.f);
				p1.y = ((fp1.x * (float)cosviewingrangeglobalang) + (fp1.y * (float)sinviewingrangeglobalang)) * (1.0f / 64.f);
			}
			else { p1 = p2; }

			p2.x = ((fp2.y * fcosglobalang) - (fp2.x * fsinglobalang)) * (1.0f / 64.f);
			p2.y = ((fp2.x * (float)cosviewingrangeglobalang) + (fp2.y * (float)sinviewingrangeglobalang)) * (1.0f / 64.f);

			if ((p1.y >= SCISDIST) || (p2.y >= SCISDIST))
				if (p1.x*p2.y < p2.x*p1.y) //if wall is facing you...
				{
					if (p1.y >= SCISDIST)
						dxb1[numscans] = p1.x*ghalfx / p1.y + ghalfx;
					else dxb1[numscans] = -1e32f;

					if (p2.y >= SCISDIST)
						dxb2[numscans] = p2.x*ghalfx / p2.y + ghalfx;
					else dxb2[numscans] = 1e32f;

					if (dxb1[numscans] < dxb2[numscans])
					{
						thesector[numscans] = sectnum; thewall[numscans] = z; bunchp2[numscans] = numscans + 1; numscans++;
					}
				}

			if ((wall[z].point2 < z) && (scanfirst < numscans))
			{
				bunchp2[numscans - 1] = scanfirst; scanfirst = numscans;
			}
		}

		for (int z = numscansbefore; z<numscans; z++)
			if ((wall[thewall[z]].point2 != thewall[bunchp2[z]]) || (dxb2[z] > dxb1[bunchp2[z]]))
			{
				bunchfirst[numbunches++] = bunchp2[z]; bunchp2[z] = -1;
#ifdef YAX_ENABLE
				if (scansector_retfast)
					return;
#endif
			}

		for (int z = bunchfrst; z<numbunches; z++)
		{
			int zz;
			for (zz = bunchfirst[z]; bunchp2[zz] >= 0; zz = bunchp2[zz]);
			bunchlast[z] = zz;
		}
	} while (sectorbordercnt > 0);
}

/*Init viewport boundary (must be 4 point convex loop):
//      (px[0],py[0]).----.(px[1],py[1])
//                  /      \
//                /          \
// (px[3],py[3]).--------------.(px[2],py[2])
*/

static void polymost_initmosts(const float * px, const float * py, int const n)
{
	if (n < 3) return;

	int32_t imin = (px[1] < px[0]);

	for (int i = n - 1; i >= 2; i--) if (px[i] < px[imin]) imin = i;

	int32_t vcnt = 1; //0 is dummy solid node

	vsp[vcnt].x = px[imin];
	vsp[vcnt].cy[0] = vsp[vcnt].fy[0] = py[imin];
	vcnt++;

	int i = imin + 1, j = imin - 1;
	if (i >= n) i = 0;
	if (j < 0) j = n - 1;

	do
	{
		if (px[i] < px[j])
		{
			if ((vcnt > 1) && (px[i] <= vsp[vcnt - 1].x)) vcnt--;
			vsp[vcnt].x = px[i];
			vsp[vcnt].cy[0] = py[i];
			int k = j + 1; if (k >= n) k = 0;
			//(px[k],py[k])
			//(px[i],?)
			//(px[j],py[j])
			vsp[vcnt].fy[0] = (px[i] - px[k])*(py[j] - py[k]) / (px[j] - px[k]) + py[k];
			vcnt++;
			i++; if (i >= n) i = 0;
		}
		else if (px[j] < px[i])
		{
			if ((vcnt > 1) && (px[j] <= vsp[vcnt - 1].x)) vcnt--;
			vsp[vcnt].x = px[j];
			vsp[vcnt].fy[0] = py[j];
			int k = i - 1; if (k < 0) k = n - 1;
			//(px[k],py[k])
			//(px[j],?)
			//(px[i],py[i])
			vsp[vcnt].cy[0] = (px[j] - px[k])*(py[i] - py[k]) / (px[i] - px[k]) + py[k];
			vcnt++;
			j--; if (j < 0) j = n - 1;
		}
		else
		{
			if ((vcnt > 1) && (px[i] <= vsp[vcnt - 1].x)) vcnt--;
			vsp[vcnt].x = px[i];
			vsp[vcnt].cy[0] = py[i];
			vsp[vcnt].fy[0] = py[j];
			vcnt++;
			i++; if (i >= n) i = 0; if (i == j) break;
			j--; if (j < 0) j = n - 1;
		}
	} while (i != j);

	if (px[i] > vsp[vcnt - 1].x)
	{
		vsp[vcnt].x = px[i];
		vsp[vcnt].cy[0] = vsp[vcnt].fy[0] = py[i];
		vcnt++;
	}

	vsp_finalize_init(vcnt);
	gtag = vcnt;
}

void Build3DBoardPolymost::editorfunc(void)
{
	const float ratio = (r_usenewaspect ? (fxdim / fydim) / (320.f / 240.f) : 1.f)  * (1.f / get_projhack_ratio());

	vec3f_t tvect = { (searchx - ghalfx) * ratio, (searchy - ghoriz) * ratio, ghalfx };

	//Tilt rotation
	vec3f_t o = { tvect.x * gctang + tvect.y * gstang, tvect.y * gctang - tvect.x * gstang, tvect.z };

	//Up/down rotation
	tvect.x = o.z*gchang - o.y*gshang;
	tvect.y = o.x;
	tvect.z = o.y*gchang + o.z*gshang;

	//Standard Left/right rotation
	vec3_t v = { Blrintf(tvect.x * fcosglobalang - tvect.y * fsinglobalang),
		Blrintf(tvect.x * fsinglobalang + tvect.y * fcosglobalang), Blrintf(tvect.z * 16384.f) };

	vec3_t vect = { globalposx, globalposy, globalposz };

	hitdata_t *hit = &polymost_hitdata;

	hitallsprites = 1;

	hitscan((const vec3_t *)&vect, globalcursectnum, //Start position
		v.x >> 10, v.y >> 10, v.z >> 6, hit, 0xffff0030);

	if (hit->sect != -1) // if hitsect is -1, hitscan overflowed somewhere
	{
		int32_t cz, fz;
		getzsofslope(hit->sect, hit->pos.x, hit->pos.y, &cz, &fz);
		hitallsprites = 0;

		searchsector = hit->sect;
		if (hit->pos.z<cz) searchstat = 1;
		else if (hit->pos.z>fz) searchstat = 2;
		else if (hit->wall >= 0)
		{
			searchbottomwall = searchwall = hit->wall; searchstat = 0;
			if (wall[hit->wall].nextwall >= 0)
			{
				getzsofslope(wall[hit->wall].nextsector, hit->pos.x, hit->pos.y, &cz, &fz);
				if (hit->pos.z > fz)
				{
					searchisbottom = 1;
					if (wall[hit->wall].cstat & 2) //'2' bottoms of walls
						searchbottomwall = wall[hit->wall].nextwall;
				}
				else
				{
					searchisbottom = 0;
					if ((hit->pos.z > cz) && (wall[hit->wall].cstat&(16 + 32))) //masking or 1-way
						searchstat = 4;
				}
			}
		}
		else if (hit->sprite >= 0) { searchwall = hit->sprite; searchstat = 3; }
		else
		{
			getzsofslope(hit->sect, hit->pos.x, hit->pos.y, &cz, &fz);
			if ((hit->pos.z << 1) < cz + fz) searchstat = 1; else searchstat = 2;
			//if (vz < 0) searchstat = 1; else searchstat = 2; //Won't work for slopes :/
		}

		if (preview_mouseaim)
		{
			if (spritesortcnt == MAXSPRITESONSCREEN)
				spritesortcnt--;

			tspritetype *tsp = &tsprite[spritesortcnt];
			double dadist, x, y, z;
			Bmemcpy(tsp, &hit->pos, sizeof(vec3_t));
			x = tsp->x - globalposx; y = tsp->y - globalposy; z = (tsp->z - globalposz) / 16.0;
			dadist = Bsqrt(x*x + y*y + z*z);
			tsp->sectnum = hit->sect;
			tsp->picnum = 2523;  // CROSSHAIR
			tsp->cstat = 128;

			if (hit->wall != -1)
			{
				tsp->cstat |= 16;
				int const ang = getangle(wall[hit->wall].x - POINT2(hit->wall).x, wall[hit->wall].y - POINT2(hit->wall).y);
				tsp->ang = ang + 512;

				vec2_t const offs = { sintable[(ang + 1024) & 2047] >> 11,
					sintable[(ang + 512) & 2047] >> 11 };

				tsp->x -= offs.x;
				tsp->y -= offs.y;

			}
			else if (hit->sprite == -1 && (hit->pos.z == sector[hit->sect].floorz || hit->pos.z == sector[hit->sect].ceilingz))
			{
				tsp->cstat = 32;
				tsp->ang = getangle(hit->pos.x - globalposx, hit->pos.y - globalposy);
			}
			else if (hit->sprite >= 0)
			{
				if (sprite[hit->sprite].cstat & 16)
				{
					tsp->cstat |= 16;
					tsp->ang = sprite[hit->sprite].ang;
				}

				else tsp->ang = (globalang + 1024) & 2047;

				vec2_t const offs = { sintable[(tsp->ang + 1536) & 2047] >> 11,
					sintable[(tsp->ang + 1024) & 2047] >> 11 };

				tsp->x -= offs.x;
				tsp->y -= offs.y;
			}
			static int lastupdate = 0;
			static int shd = 30;
			static int shdinc = 1;

			if (totalclock > lastupdate)
			{
				shd += shdinc;
				if (shd >= 30 || shd <= 0)
				{
					shdinc = -shdinc;
					shd += shdinc;
				}
				lastupdate = totalclock + 3;
			}

			tsp->shade = 30 - shd;
			tsp->owner = MAXSPRITES - 1;
			tsp->xrepeat = tsp->yrepeat = min(max(1, (int32_t)(dadist*((double)(shd * 3) / 3200.0))), 255);
			sprite[tsp->owner].xoffset = sprite[tsp->owner].yoffset = 0;
			tspriteptr[spritesortcnt++] = tsp;
		}

		if ((searchstat == 1 || searchstat == 2) && searchsector >= 0)
		{
			vec2_t const scrv = { (v.x >> 12), (v.y >> 12) };
			vec2_t const scrv_r = { scrv.y, -scrv.x };
			walltype const * const wal = &wall[sector[searchsector].wallptr];
			uint64_t bestwdistsq = 0x7fffffff;
			int32_t bestk = -1;

			for (int32_t k = 0; k < sector[searchsector].wallnum; k++)
			{
				vec2_t const w1 = { wal[k].x, wal[k].y };
				vec2_t const w2 = { wall[wal[k].point2].x, wall[wal[k].point2].y };
				vec2_t const w21 = { w1.x - w2.x, w1.y - w2.y };
				vec2_t const pw1 = { w1.x - hit->pos.x, w1.y - hit->pos.y };
				vec2_t const pw2 = { w2.x - hit->pos.x, w2.y - hit->pos.y };
				float w1d = (float)(scrv_r.x * pw1.x + scrv_r.y * pw1.y);
				float w2d = (float)-(scrv_r.x * pw2.x + scrv_r.y * pw2.y);

				if ((w1d == 0 && w2d == 0) || (w1d < 0 || w2d < 0))
					continue;

				vec2_t const ptonline = { (int32_t)(w2.x + (w2d / (w1d + w2d)) * w21.x),
					(int32_t)(w2.y + (w2d / (w1d + w2d)) * w21.y) };

				vec2_t const scrp = { ptonline.x - vect.x, ptonline.y - vect.y };

				if (scrv.x * scrp.x + scrv.y * scrp.y <= 0)
					continue;

				int64_t const t1 = scrp.x;
				int64_t const t2 = scrp.y;

				uint64_t const wdistsq = t1 * t1 + t2 * t2;

				if (wdistsq < bestwdistsq)
				{
					bestk = k;
					bestwdistsq = wdistsq;
				}
			}

			if (bestk >= 0)
				searchwall = sector[searchsector].wallptr + bestk;
		}
	}
	searchit = 0;
}


void Build3DBoardPolymost::drawrooms()
{
	numDrawPolyCommands = 0;
	memset(&visibleSectorList[0], 0, sizeof(bool) * MAXSECTORS);

//	begindrawing();
	frameoffset = frameplace + windowy1*bytesperline + windowx1;

	resizeglcheck();

	//Polymost supports true look up/down :) Here, we convert horizon to angle.
	//gchang&gshang are cos&sin of this angle (respectively)
	fviewingrange = (float)viewingrange;
	gyxscale = ((float)xdimenscale)*(1.0f / 131072.f);
	gxyaspect = ((float)xyaspect*fviewingrange)*(5.f / (65536.f*262144.f));
	gviewxrange = fviewingrange * fxdimen * (1.f / (32768.f*1024.f));
	fcosglobalang = (float)cosglobalang;
	gcosang = fcosglobalang*(1.0f / 262144.f);
	fsinglobalang = (float)singlobalang;
	gsinang = fsinglobalang*(1.0f / 262144.f);
	gcosang2 = gcosang * (fviewingrange * (1.0f / 65536.f));
	gsinang2 = gsinang * (fviewingrange * (1.0f / 65536.f));
	ghalfx = (float)(xdimen >> 1);
	grhalfxdown10 = 1.f / (ghalfx*1024.f);
	ghoriz = (float)globalhoriz;

	gvisibility = ((float)globalvisibility)*FOGSCALE;

	//global cos/sin height angle
	float r = (float)(ydimen >> 1) - ghoriz;
	gshang = r / Bsqrtf(r*r + ghalfx*ghalfx);
	gchang = Bsqrtf(1.f - gshang*gshang);
	ghoriz = (float)(ydimen >> 1);

	//global cos/sin tilt angle
	gctang = cosf(gtang);
	gstang = sinf(gtang);

	if (Bfabsf(gstang) < .001f)  // This hack avoids nasty precision bugs in domost()
	{
		gstang = 0.f;
		gctang = (gctang > 0.f) ? 1.f : -1.f;
	}

	if (inpreparemirror)
		gstang = -gstang;

	//Generate viewport trapezoid (for handling screen up/down)
	vec3f_t p[4] = { { 0 - 1,                                  0 - 1,                                  0 },
	{ (float)(windowx2 + 1 - windowx1 + 2), 0 - 1,                                  0 },
	{ (float)(windowx2 + 1 - windowx1 + 2), (float)(windowy2 + 1 - windowy1 + 2), 0 },
	{ 0 - 1,                                  (float)(windowy2 + 1 - windowy1 + 2), 0 } };

	for (int i = 0; i<4; i++)
	{
		//Tilt rotation (backwards)
		vec2f_t const o = { p[i].x - ghalfx, p[i].y - ghoriz };
		vec3f_t const o2 = { o.x*gctang + o.y*gstang, o.y*gctang - o.x*gstang, ghalfx };

		//Up/down rotation (backwards)
		p[i].x = o2.x;
		p[i].y = o2.y*gchang + o2.z*gshang;
		p[i].z = o2.z*gchang - o2.y*gshang;
	}

	//Clip to SCISDIST plane
	int n = 0;

	vec3f_t p2[6];

	for (int i = 0; i<4; i++)
	{
		int const j = i < 3 ? i + 1 : 0;

		if (p[i].z >= SCISDIST)
			p2[n++] = p[i];

		if ((p[i].z >= SCISDIST) != (p[j].z >= SCISDIST))
		{
			float const r = (SCISDIST - p[i].z) / (p[j].z - p[i].z);
			p2[n].x = (p[j].x - p[i].x) * r + p[i].x;
			p2[n].y = (p[j].y - p[i].y) * r + p[i].y;
			p2[n].z = SCISDIST; n++;
		}
	}

	if (n < 3) { enddrawing(); return; }

	float sx[4], sy[4];

	for (int i = 0; i < n; i++)
	{
		float const r = ghalfx / p2[i].z;
		sx[i] = p2[i].x * r + ghalfx;
		sy[i] = p2[i].y * r + ghoriz;
	}

	polymost_initmosts(sx, sy, n);

	if (searchit == 2)
		editorfunc();

	numscans = numbunches = 0;

	// MASKWALL_BAD_ACCESS
	// Fixes access of stale maskwall[maskwallcnt] (a "scan" index, in BUILD lingo):
	maskwallcnt = 0;

	// NOTE: globalcursectnum has been already adjusted in ADJUST_GLOBALCURSECTNUM.
	Bassert((unsigned)globalcursectnum < MAXSECTORS);
	scansector(globalcursectnum);

	grhalfxdown10x = grhalfxdown10;

	if (inpreparemirror)
	{
		grhalfxdown10x = -grhalfxdown10;
		inpreparemirror = 0;

		// see engine.c: INPREPAREMIRROR_NO_BUNCHES
		if (numbunches > 0)
		{
			drawalls(0);
			numbunches--;
			bunchfirst[0] = bunchfirst[numbunches];
			bunchlast[0] = bunchlast[numbunches];
		}
	}

	while (numbunches > 0)
	{
		Bmemset(ptempbuf, 0, numbunches + 3); ptempbuf[0] = 1;

		int32_t closest = 0;              //Almost works, but not quite :(

		for (int i = 1; i<numbunches; ++i)
		{
			int const bnch = bunchfront(i, closest); if (bnch < 0) continue;
			ptempbuf[i] = 1;
			if (!bnch) { ptempbuf[closest] = 1; closest = i; }
		}
		for (int i = 0; i<numbunches; ++i) //Double-check
		{
			if (ptempbuf[i]) continue;
			int const bnch = bunchfront(i, closest); if (bnch < 0) continue;
			ptempbuf[i] = 1;
			if (!bnch) { ptempbuf[closest] = 1; closest = i; i = 0; }
		}

		drawalls(closest);

		numbunches--;
		bunchfirst[closest] = bunchfirst[numbunches];
		bunchlast[closest] = bunchlast[numbunches];
	}

//	enddrawing();
}

void Build3DBoardPolymost::drawmaskwall(int32_t damaskwallcnt)
{
	int const z = maskwall[damaskwallcnt];
	twalltype const * const wal = (twalltype *)&wall[thewall[z]], *wal2 = (twalltype *)&wall[wal->point2];
	int32_t const sectnum = thesector[z];
	tsectortype const * const sec = (tsectortype *)&sector[sectnum];

	//    if (wal->nextsector < 0) return;
	// Without MASKWALL_BAD_ACCESS fix:
	// wal->nextsector is -1, WGR2 SVN Lochwood Hollow (Til' Death L1)  (or trueror1.map)

	tsectortype const * const nsec = (tsectortype *)&sector[wal->nextsector];

	AddUniqueSector(sectnum);
	AddUniqueSector(wal->nextsector);

	globalpicnum = wal->overpicnum;
	if ((uint32_t)globalpicnum >= MAXTILES)
		globalpicnum = 0;

	DO_TILE_ANIM(globalpicnum, (int16_t)thewall[z] + 16384);

	globvis = (sector[sectnum].visibility != 0) ? mulscale4(globvis, (uint8_t)(sector[sectnum].visibility + 16)) : globalvisibility;

	globalshade = (int32_t)wal->shade;
	globalpal = (int32_t)((uint8_t)wal->pal);
	globalorientation = (int32_t)wal->cstat;

	vec2f_t s0 = { (float)(wal->x - globalposx), (float)(wal->y - globalposy) };
	vec2f_t p0 = { s0.y*gcosang - s0.x*gsinang, s0.x*gcosang2 + s0.y*gsinang2 };

	vec2f_t s1 = { (float)(wal2->x - globalposx), (float)(wal2->y - globalposy) };
	vec2f_t p1 = { s1.y*gcosang - s1.x*gsinang, s1.x*gcosang2 + s1.y*gsinang2 };

	if ((p0.y < SCISDIST) && (p1.y < SCISDIST)) return;

	//Clip to close parallel-screen plane
	vec2f_t const op0 = p0;

	float t0 = 0.f;

	if (p0.y < SCISDIST)
	{
		t0 = (SCISDIST - p0.y) / (p1.y - p0.y);
		p0.x = (p1.x - p0.x) * t0 + p0.x;
		p0.y = SCISDIST;
	}

	float t1 = 1.f;

	if (p1.y < SCISDIST)
	{
		t1 = (SCISDIST - op0.y) / (p1.y - op0.y);
		p1.x = (p1.x - op0.x) * t1 + op0.x;
		p1.y = SCISDIST;
	}

	int32_t m0 = (int32_t)((wal2->x - wal->x) * t0 + wal->x);
	int32_t m1 = (int32_t)((wal2->y - wal->y) * t0 + wal->y);
	int32_t cz[4], fz[4];
	getzsofslope(sectnum, m0, m1, &cz[0], &fz[0]);
	getzsofslope(wal->nextsector, m0, m1, &cz[1], &fz[1]);
	m0 = (int32_t)((wal2->x - wal->x) * t1 + wal->x);
	m1 = (int32_t)((wal2->y - wal->y) * t1 + wal->y);
	getzsofslope(sectnum, m0, m1, &cz[2], &fz[2]);
	getzsofslope(wal->nextsector, m0, m1, &cz[3], &fz[3]);

	float ryp0 = 1.f / p0.y;
	float ryp1 = 1.f / p1.y;

	//Generate screen coordinates for front side of wall
	float const x0 = ghalfx*p0.x*ryp0 + ghalfx;
	float const x1 = ghalfx*p1.x*ryp1 + ghalfx;
	if (x1 <= x0) return;

	ryp0 *= gyxscale; ryp1 *= gyxscale;

	xtex.d = (ryp0 - ryp1)*gxyaspect / (x0 - x1);
	ytex.d = 0;
	otex.d = ryp0*gxyaspect - xtex.d*x0;

	//gux*x0 + guo = t0*wal->xrepeat*8*yp0
	//gux*x1 + guo = t1*wal->xrepeat*8*yp1
	xtex.u = (t0*ryp0 - t1*ryp1)*gxyaspect*(float)wal->xrepeat*8.f / (x0 - x1);
	otex.u = t0*ryp0*gxyaspect*(float)wal->xrepeat*8.f - xtex.u*x0;
	otex.u += (float)wal->xpanning*otex.d;
	xtex.u += (float)wal->xpanning*xtex.d;
	ytex.u = 0;

	// mask
	calc_ypanning((!(wal->cstat & 4)) ? max(nsec->ceilingz, sec->ceilingz) : min(nsec->floorz, sec->floorz), ryp0, ryp1,
		x0, x1, wal->ypanning, wal->yrepeat, 0);

	if (wal->cstat & 8) //xflip
	{
		float const t = (float)(wal->xrepeat * 8 + wal->xpanning * 2);
		xtex.u = xtex.d*t - xtex.u;
		ytex.u = ytex.d*t - ytex.u;
		otex.u = otex.d*t - otex.u;
	}
	if (wal->cstat & 256) { xtex.v = -xtex.v; ytex.v = -ytex.v; otex.v = -otex.v; } //yflip

	int method = 1 | DAMETH_WALL;

	if (wal->cstat & 128)
	{
		if (!(wal->cstat & 512))
			method = 2 | DAMETH_WALL;
		else
			method = 3 | DAMETH_WALL;
	}

	//if (!nofog)
	//	calc_and_apply_fog(wal->picnum, fogpal_shade(sec, wal->shade), sec->visibility, get_floor_fogpal(sec));

	float const csy[4] = { ((float)(cz[0] - globalposz)) * ryp0 + ghoriz,
		((float)(cz[1] - globalposz)) * ryp0 + ghoriz,
		((float)(cz[2] - globalposz)) * ryp1 + ghoriz,
		((float)(cz[3] - globalposz)) * ryp1 + ghoriz };

	float const fsy[4] = { ((float)(fz[0] - globalposz)) * ryp0 + ghoriz,
		((float)(fz[1] - globalposz)) * ryp0 + ghoriz,
		((float)(fz[2] - globalposz)) * ryp1 + ghoriz,
		((float)(fz[3] - globalposz)) * ryp1 + ghoriz };

	//Clip 2 quadrilaterals
	//               /csy3
	//             /   |
	// csy0------/----csy2
	//   |     /xxxxxxx|
	//   |   /xxxxxxxxx|
	// csy1/xxxxxxxxxxx|
	//   |xxxxxxxxxxx/fsy3
	//   |xxxxxxxxx/   |
	//   |xxxxxxx/     |
	// fsy0----/------fsy2
	//   |   /
	// fsy1/

	vec2f_t dpxy[4] = { { x0, csy[1] },{ x1, csy[3] },{ x1, fsy[3] },{ x0, fsy[1] } };

	//Clip to (x0,csy[0])-(x1,csy[2])

	vec2f_t dp2[4];

	int n2 = 0;
	t1 = -((dpxy[0].x - x0) * (csy[2] - csy[0]) - (dpxy[0].y - csy[0]) * (x1 - x0));

	for (int i = 0; i<4; i++)
	{
		int j = i + 1;

		if (j >= 4)
			j = 0;

		t0 = t1;
		t1 = -((dpxy[j].x - x0) * (csy[2] - csy[0]) - (dpxy[j].y - csy[0]) * (x1 - x0));

		if (t0 >= 0)
			dp2[n2++] = dpxy[i];

		if ((t0 >= 0) != (t1 >= 0))
		{
			float const r = t0 / (t0 - t1);
			dp2[n2].x = (dpxy[j].x - dpxy[i].x) * r + dpxy[i].x;
			dp2[n2].y = (dpxy[j].y - dpxy[i].y) * r + dpxy[i].y;
			n2++;
		}
	}

	if (n2 < 3)
		return;

	//Clip to (x1,fsy[2])-(x0,fsy[0])
	t1 = -((dp2[0].x - x1) * (fsy[0] - fsy[2]) - (dp2[0].y - fsy[2]) * (x0 - x1));
	int n = 0;

	for (int i = 0, j = 1; i < n2; j = ++i + 1)
	{
		if (j >= n2)
			j = 0;

		t0 = t1;
		t1 = -((dp2[j].x - x1) * (fsy[0] - fsy[2]) - (dp2[j].y - fsy[2]) * (x0 - x1));

		if (t0 >= 0)
			dpxy[n++] = dp2[i];

		if ((t0 >= 0) != (t1 >= 0))
		{
			float const r = t0 / (t0 - t1);
			dpxy[n].x = (dp2[j].x - dp2[i].x) * r + dp2[i].x;
			dpxy[n].y = (dp2[j].y - dp2[i].y) * r + dp2[i].y;
			n++;
		}
	}

	if (n < 3)
		return;

	pow2xsplit = 0;
	//skyclamphack = 0;
	//drawpoly_alpha = 0.f;
	drawpoly(dpxy, n, method);
}

typedef struct
{
	uint32_t wrev;
	uint32_t srev;
	int16_t wall;
	int8_t wdist;
	int8_t filler;
} wallspriteinfo_t;

wallspriteinfo_t wsprinfo[MAXSPRITES];

int32_t Build3DBoardPolymost::findwall(tspritetype const * const tspr, int32_t * rd)
{
	int32_t dist = 4, closest = -1;
	tsectortype const * const sect = (tsectortype  *)&sector[tspr->sectnum];
	vec2_t n;

	for (int i = sect->wallptr; i<sect->wallptr + sect->wallnum; i++)
	{
		if (!getclosestpointonwall((const vec2_t *)tspr, i, &n))
		{
			int const dst = klabs(tspr->x - n.x) + klabs(tspr->y - n.y);

			if (dst <= dist)
			{
				dist = dst;
				closest = i;
			}
		}
	}

	*rd = dist;

	return closest;
}

int32_t Build3DBoardPolymost::lintersect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, int32_t x4, int32_t y4)
{
	// p1 to p2 is a line segment
	int32_t const x21 = x2 - x1, x34 = x3 - x4;
	int32_t const y21 = y2 - y1, y34 = y3 - y4;
	int32_t const bot = x21 * y34 - y21 * x34;

	if (!bot)
		return 0;

	int32_t const x31 = x3 - x1, y31 = y3 - y1;
	int32_t const topt = x31 * y34 - y31 * x34;

	int rv = 1;

	if (bot > 0)
	{
		if ((unsigned)topt >= (unsigned)bot)
			rv = 0;

		int32_t topu = x21 * y31 - y21 * x31;

		if ((unsigned)topu >= (unsigned)bot)
			rv = 0;
	}
	else
	{
		if ((unsigned)topt <= (unsigned)bot)
			rv = 0;

		int32_t topu = x21 * y31 - y21 * x31;

		if ((unsigned)topu <= (unsigned)bot)
			rv = 0;
	}

	return rv;
}
