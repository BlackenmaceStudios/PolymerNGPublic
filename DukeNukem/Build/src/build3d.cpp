// build3d.cpp
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
#include "PolymerNG/PolymerNG.h"
#include "PolymerNG/Models/Models.h"
#include "PolymerNG/Renderer/Renderer.h"

#pragma optimize( "", off )
Build3D	build3D;

/*
==========================================================

Build3D

This is a API agnostic version of vertex processing functions needed to translate build content into 3D graphics. This entire system relies on global variables that are shared between functions,
still poorly coded(copy and paste of the original code), todo fix me.

This code is grabbed from Polymost and Polymer and heavily modified.
==========================================================
*/

int32_t rendmode = 0;
#ifdef EDUKE32_GLES
int32_t usemodels = 0;
#else
int32_t usemodels = 1;
#endif
int32_t usehightile = 1;
int32_t vsync = 0;

const Build3DPlane		*renderPlanesGlobalPool[MAX_CONCURRENT_DRAWBOARDS][60000];
const Build3DPlane		*renderPlanesGlobalPool2[MAX_CONCURRENT_DRAWBOARDS][60000];

#include <math.h> //<-important!
#include <float.h>

typedef struct { float x, cy[2], fy[2]; int32_t tag; int16_t n, p, ctag, ftag; } vsptyp;
#define VSPMAX 4096 //<- careful!
static vsptyp vsp[VSPMAX];
static int32_t gtag;
bool pow2xsplit = false;

static float dxb1[MAXWALLSB], dxb2[MAXWALLSB];

#define SCISDIST 1.0f  //1.0: Close plane clipping distance

float shadescale = 1.0f;
int32_t shadescale_unbounded = 0;

int32_t r_usenewshading = 3;
int32_t r_usetileshades = 2;
int32_t r_npotwallmode = 0;

static float gviewxrange;
static float ghoriz;
float gxyaspect;
float gyxscale, ghalfx, grhalfxdown10, grhalfxdown10x;
float gcosang, gsinang, gcosang2, gsinang2;
float gchang, gshang, gctang, gstang, gvisibility;
float gtang = 0.f;

static vec3d_t xtex, ytex, otex;

float fcosglobalang, fsinglobalang;
float fxdim, fydim, fydimen, fviewingrange;
static int32_t preview_mouseaim = 1;  // when 1, displays a CROSSHAIR tsprite at the _real_ aimed position

static int32_t drawpoly_srepeat = 0, drawpoly_trepeat = 0;

#ifdef REDBLUEMODE
int32_t glredbluemode = 0;
static int32_t lastglredbluemode = 0, redblueclearcnt = 0;
#endif



int32_t glanisotropy = 1;            // 0 = maximum supported by card
									 //int32_t gltexfiltermode = TEXFILTER_OFF;

#ifdef EDUKE32_GLES
int32_t glusetexcompr = 0;
int32_t glusetexcache = 0, glusememcache = 0;
#else
int32_t glusetexcompr = 1;
int32_t glusetexcache = 2, glusememcache = 1;
int32_t r_polygonmode = 0;     // 0:GL_FILL,1:GL_LINE,2:GL_POINT //FUK
int32_t glmultisample = 0, glnvmultisamplehint = 0;
static int32_t lastglpolygonmode = 0; //FUK
int32_t r_detailmapping = 1;
int32_t r_glowmapping = 1;
#endif

int32_t gltexmaxsize = 0;      // 0 means autodetection on first run
int32_t gltexmiplevel = 0;		// discards this many mipmap levels
int32_t glprojectionhacks = 1;
//static GLuint polymosttext = 0;
int32_t glrendmode = REND_POLYMOST;

// This variable, and 'shadeforfullbrightpass' control the drawing of
// fullbright tiles.  Also see 'fullbrightloadingpass'.

int32_t r_fullbrights = 1;
int32_t r_vertexarrays = 1;
int32_t r_vbos = 1;
int32_t r_vbocount = 64;
int32_t r_animsmoothing = 1;
int32_t r_downsize = 0;
int32_t r_downsizevar = -1;

// used for fogcalc
static float fogresult, fogresult2;
//coltypef fogcol, fogtable[MAXPALOOKUPS];

static const float float_trans[4] = { 1.0f, 1.0f, 0.66f, 0.33f };

char ptempbuf[MAXWALLSB << 1];

// polymost ART sky control
int32_t r_parallaxskyclamping = 1;
int32_t r_parallaxskypanning = 0;

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

int32_t mdtims, omdtims;
uint8_t alphahackarray[MAXTILES];
int32_t drawingskybox = 0;
int32_t hicprecaching = 0;

hitdata_t polymost_hitdata;

#define INDICE(n) ((p->indices) ? (p->indices[(i+n)%p->indicescount]) : (((i+n)%p->vertcount)))


#define USE_POLYMER_LEGACY_MATH

#ifndef USE_POLYMER_LEGACY_MATH
#define INVERT_3X3(b,det,a) 
#else
/* ========================================================== */
/* determinant of matrix
*
* Computes determinant of matrix m, returning d
*/

#define DETERMINANT_3X3(d,m)                    \
{                                \
   d = m[0][0] * (m[1][1]*m[2][2] - m[1][2] * m[2][1]);        \
   d -= m[0][1] * (m[1][0]*m[2][2] - m[1][2] * m[2][0]);    \
   d += m[0][2] * (m[1][0]*m[2][1] - m[1][1] * m[2][0]);    \
}

/* ========================================================== */
/* i,j,th cofactor of a 4x4 matrix
*
*/

#define COFACTOR_4X4_IJ(fac,m,i,j)                 \
{                                \
   int ii[4], jj[4], k;                        \
                                \
   /* compute which row, columnt to skip */            \
   for (k=0; k<i; k++) ii[k] = k;                \
   for (k=i; k<3; k++) ii[k] = k+1;                \
   for (k=0; k<j; k++) jj[k] = k;                \
   for (k=j; k<3; k++) jj[k] = k+1;                \
                                \
   (fac) = m[ii[0]][jj[0]] * (m[ii[1]][jj[1]]*m[ii[2]][jj[2]]     \
                            - m[ii[1]][jj[2]]*m[ii[2]][jj[1]]); \
   (fac) -= m[ii[0]][jj[1]] * (m[ii[1]][jj[0]]*m[ii[2]][jj[2]]    \
                             - m[ii[1]][jj[2]]*m[ii[2]][jj[0]]);\
   (fac) += m[ii[0]][jj[2]] * (m[ii[1]][jj[0]]*m[ii[2]][jj[1]]    \
                             - m[ii[1]][jj[1]]*m[ii[2]][jj[0]]);\
                                \
   /* compute sign */                        \
   k = i+j;                            \
   if ( k != (k/2)*2) {                        \
      (fac) = -(fac);                        \
   }                                \
}

/* ========================================================== */
/* determinant of matrix
*
* Computes determinant of matrix m, returning d
*/

#define DETERMINANT_4X4(d,m)                    \
{                                \
   double cofac;                        \
   COFACTOR_4X4_IJ (cofac, m, 0, 0);                \
   d = m[0][0] * cofac;                        \
   COFACTOR_4X4_IJ (cofac, m, 0, 1);                \
   d += m[0][1] * cofac;                    \
   COFACTOR_4X4_IJ (cofac, m, 0, 2);                \
   d += m[0][2] * cofac;                    \
   COFACTOR_4X4_IJ (cofac, m, 0, 3);                \
   d += m[0][3] * cofac;                    \
}

/* ========================================================== */
/* compute adjoint of matrix and scale
*
* Computes adjoint of matrix m, scales it by s, returning a
*/

#define SCALE_ADJOINT_3X3(a,s,m)                \
{                                \
   a[0][0] = (s) * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);    \
   a[1][0] = (s) * (m[1][2] * m[2][0] - m[1][0] * m[2][2]);    \
   a[2][0] = (s) * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);    \
                                \
   a[0][1] = (s) * (m[0][2] * m[2][1] - m[0][1] * m[2][2]);    \
   a[1][1] = (s) * (m[0][0] * m[2][2] - m[0][2] * m[2][0]);    \
   a[2][1] = (s) * (m[0][1] * m[2][0] - m[0][0] * m[2][1]);    \
                                \
   a[0][2] = (s) * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);    \
   a[1][2] = (s) * (m[0][2] * m[1][0] - m[0][0] * m[1][2]);    \
   a[2][2] = (s) * (m[0][0] * m[1][1] - m[0][1] * m[1][0]);    \
}

/* ========================================================== */
/* compute adjoint of matrix and scale
*
* Computes adjoint of matrix m, scales it by s, returning a
*/

#define SCALE_ADJOINT_4X4(a,s,m)                \
{                                \
   int i,j;                            \
                                \
   for (i=0; i<4; i++) {                    \
      for (j=0; j<4; j++) {                    \
         COFACTOR_4X4_IJ (a[j][i], m, i, j);            \
         a[j][i] *= s;                        \
      }                                \
   }                                \
}

/* ========================================================== */
/* inverse of matrix
*
* Compute inverse of matrix a, returning determinant m and
* inverse b
*/

#define INVERT_3X3(b,det,a)            \
{                        \
   double tmp;                    \
   DETERMINANT_3X3 (det, a);            \
   tmp = 1.0 / (det);                \
   SCALE_ADJOINT_3X3 (b, tmp, a);        \
}

/* ========================================================== */
/* inverse of matrix
*
* Compute inverse of matrix a, returning determinant m and
* inverse b
*/

#define INVERT_4X4(b,det,a)            \
{                        \
   double tmp;                    \
   DETERMINANT_4X4 (det, a);            \
   tmp = 1.0 / (det);                \
   SCALE_ADJOINT_4X4 (b, tmp, a);        \
}
#endif

//
// Build3DBoard::Build3DBoard
//
Build3DBoard::Build3DBoard()
{
	prtess = gluNewTess();
	model = new BaseModel();
}

/*
================
Build3dMath_CrossProduct

This is polymer_crossproduct
================
*/
void  Build3dMath_CrossProduct(float* in_a, float* in_b, float* out)
{
	out[0] = in_a[1] * in_b[2] - in_a[2] * in_b[1];
	out[1] = in_a[2] * in_b[0] - in_a[0] * in_b[2];
	out[2] = in_a[0] * in_b[1] - in_a[1] * in_b[0];
}

/*
================
Build3dMath_Normalize
================
*/
void  Build3dMath_Normalize(float* vec)
{
	double norm;

	norm = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];

	norm = sqrt(norm);
	norm = 1.0 / norm;
	vec[0] *= norm;
	vec[1] *= norm;
	vec[2] *= norm;
}
/*
================
Build3DBoard::initwall
================
*/
bool Build3DBoard::initwall(int16_t wallnum)
{
	Build3DWall         *w;

	w = new Build3DWall();


	if (w->mask.buffer == NULL) {
		w->mask.buffer = new Build3DVertex[4]; // (_prvert *)Xmalloc(4 * sizeof(_prvert));
		w->mask.vertcount = 4;
	}
	if (w->bigportal == NULL)
		w->bigportal = new float[20]; // (GLfloat *)Xmalloc(4 * sizeof(GLfloat) * 5);

	w->flags.empty = 1;

	prwalls[wallnum] = w;

	return true;
}

// TODO: r_npotwallmode. Needs polymost_is_npotmode() handling among others.
#define DAMETH_WALL 0
/*
================
Build3DBoard::initwall
================
*/
float Build3DBoard::calc_ypancoef(char curypanning, int16_t curpicnum, int32_t dopancor)
{
#ifdef NEW_MAP_FORMAT
	if (g_loadedMapVersion >= 10)
		return curypanning / 256.0f;
#endif
	{
		float ypancoef = (float)(pow2long[picsiz[curpicnum] >> 4]);

		if (ypancoef < tilesiz[curpicnum].y)
			ypancoef *= 2;

		if (dopancor)
		{
			int32_t yoffs = Blrintf((ypancoef - tilesiz[curpicnum].y) * (255.0f / ypancoef));
			if (curypanning > 256 - yoffs)
				curypanning -= yoffs;
		}

		ypancoef *= (float)curypanning / (256.0f * (float)tilesiz[curpicnum].y);

		return ypancoef;
	}
}

#define NBYTES_WALL_CSTAT_THROUGH_YPANNING \
    (offsetof(walltype, ypanning)+sizeof(wall[0].ypanning) - offsetof(walltype, cstat))

/*
================
Build3DBoard::updatewall
================
*/
bool Build3DBoard::updatewall(int16_t wallnum)
{
	int16_t         nwallnum, nnwallnum, curpicnum, wallpicnum, walloverpicnum, nwallpicnum;
	char            curxpanning, curypanning, underwall, overwall, curpal;
	int8_t          curshade;
	walltype        *wal;
	sectortype      *sec, *nsec;
	Build3DWall         *w;
	Build3DSector       *s, *ns;
	int32_t         xref, yref;
	float           ypancoef, dist;
	int32_t         i;
	uint32_t        invalid;
	int32_t         sectofwall = sectorofwall(wallnum);


	// yes, this function is messy and unefficient
	// it also works, bitches
	sec = &sector[sectofwall];

	if (sectofwall < 0 || sectofwall >= numsectors ||
		wallnum < 0 || wallnum > numwalls ||
		sec->wallptr > wallnum || wallnum >= (sec->wallptr + sec->wallnum))
		return false; // yay, corrupt map

	wal = &wall[wallnum];
	nwallnum = wal->nextwall;

	w = prwalls[wallnum];
	s = prsectors[sectofwall];
	invalid = s->invalidid;
	if (nwallnum >= 0 && nwallnum < numwalls && wal->nextsector >= 0 && wal->nextsector < numsectors)
	{
		ns = prsectors[wal->nextsector];
		invalid += ns->invalidid;
		nsec = &sector[wal->nextsector];
	}
	else
	{
		ns = NULL;
		nsec = NULL;
	}

	if (w->wall.buffer == NULL) {
		w->wall.buffer = new Build3DVertex[4]; // (_prvert *)Xcalloc(4, sizeof(_prvert));  // XXX
		w->wall.vertcount = 4;
	}

	wallpicnum = wal->picnum;
	DO_TILE_ANIM(wallpicnum, wallnum + 16384);

	walloverpicnum = wal->overpicnum;
	if (walloverpicnum >= 0)
		DO_TILE_ANIM(walloverpicnum, wallnum + 16384);

	if (nwallnum >= 0 && nwallnum < numwalls)
	{
		nwallpicnum = wall[nwallnum].picnum;
		DO_TILE_ANIM(nwallpicnum, wallnum + 16384);
	}
	else
		nwallpicnum = 0;

	if ((!w->flags.empty) && (!w->flags.invalidtex) &&
		(w->invalidid == invalid) &&
		(wallpicnum == w->picnum_anim) &&
		(walloverpicnum == w->overpicnum_anim) &&
		!Bmemcmp(&wal->cstat, &w->cstat, NBYTES_WALL_CSTAT_THROUGH_YPANNING) &&
		((nwallnum < 0 || nwallnum > numwalls) ||
			((nwallpicnum == w->nwallpicnum) &&
				(wall[nwallnum].xpanning == w->nwallxpanning) &&
				(wall[nwallnum].ypanning == w->nwallypanning) &&
				(wall[nwallnum].cstat == w->nwallcstat) &&
				(wall[nwallnum].shade == w->nwallshade))))
	{
		w->flags.uptodate = 1;
		return false; // screw you guys I'm going home
	}
	else
	{
		w->invalidid = invalid;

		Bmemcpy(&w->cstat, &wal->cstat, NBYTES_WALL_CSTAT_THROUGH_YPANNING);

		w->picnum_anim = wallpicnum;
		w->overpicnum_anim = walloverpicnum;

		if (nwallnum >= 0 && nwallnum < numwalls)
		{
			w->nwallpicnum = nwallpicnum;
			w->nwallxpanning = wall[nwallnum].xpanning;
			w->nwallypanning = wall[nwallnum].ypanning;
			w->nwallcstat = wall[nwallnum].cstat;
			w->nwallshade = wall[nwallnum].shade;
		}
	}

	w->underover = underwall = overwall = 0;

	if (wal->cstat & 8)
		xref = 1;
	else
		xref = 0;

	if ((unsigned)wal->nextsector >= (unsigned)numsectors || !ns)
	{
		// jmarshall - nasty memcpy replacement
		//		Bmemcpy(w->wall.buffer, &s->floor.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 3);
		//		Bmemcpy(&w->wall.buffer[1], &s->floor.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 3);
		//		Bmemcpy(&w->wall.buffer[2], &s->ceil.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 3);
		//		Bmemcpy(&w->wall.buffer[3], &s->ceil.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 3);
		w->wall.buffer[0].position = s->floor.buffer[wallnum - sec->wallptr].position;
		w->wall.buffer[1].position = s->floor.buffer[wal->point2 - sec->wallptr].position;
		w->wall.buffer[2].position = s->ceil.buffer[wal->point2 - sec->wallptr].position;
		w->wall.buffer[3].position = s->ceil.buffer[wallnum - sec->wallptr].position;
		// jmarshall end

		if (wal->nextsector < 0)
			curpicnum = wallpicnum;
		else
			curpicnum = walloverpicnum;

		//w->wall.bucket = polymer_getbuildmaterial(&w->wall.material, curpicnum, wal->pal, wal->shade, sec->visibility, DAMETH_WALL);

		if (wal->cstat & 4)
			yref = sec->floorz;
		else
			yref = sec->ceilingz;

		if ((wal->cstat & 32) && (wal->nextsector >= 0))
		{
			if ((!(wal->cstat & 2) && (wal->cstat & 4)) || ((wal->cstat & 2) && (wall[nwallnum].cstat & 4)))
				yref = sec->ceilingz;
			else
				yref = nsec->floorz;
		}

		if (wal->ypanning)
			// white (but not 1-way)
			ypancoef = calc_ypancoef(wal->ypanning, curpicnum, !(wal->cstat & 4));
		else
			ypancoef = 0;

		i = 0;
		while (i < 4)
		{
			if ((i == 0) || (i == 3))
				dist = (float)xref;
			else
				dist = (float)(xref == 0);

			w->wall.buffer[i].uv.x = ((dist * 8.0f * wal->xrepeat) + wal->xpanning) / (float)(tilesiz[curpicnum].x);
			w->wall.buffer[i].uv.y = (-(float)(yref + (w->wall.buffer[i].position.y * 16)) / ((tilesiz[curpicnum].y * 2048.0f) / (float)(wal->yrepeat))) + ypancoef;

			if (wal->cstat & 256) w->wall.buffer[i].uv.y = -w->wall.buffer[i].uv.y;

			i++;
		}

		w->underover |= 1;
	}
	else
	{
		nnwallnum = wall[nwallnum].point2;

		if ((s->floor.buffer[wallnum - sec->wallptr].position.y < ns->floor.buffer[nnwallnum - nsec->wallptr].position.y) ||
			(s->floor.buffer[wal->point2 - sec->wallptr].position.y < ns->floor.buffer[nwallnum - nsec->wallptr].position.y))
			underwall = 1;

		if ((underwall) || (wal->cstat & 16) || (wal->cstat & 32))
		{
			int32_t refwall;

			if (s->floor.buffer[wallnum - sec->wallptr].position.y < ns->floor.buffer[nnwallnum - nsec->wallptr].position.y)
			{
				// jmarshall - nasty memcpy replacement.
				//				Bmemcpy(w->wall.buffer, &s->floor.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 3);
				w->wall.buffer[0].position = s->floor.buffer[wallnum - sec->wallptr].position;
				// jmarshall end
			}
			else
			{
				// jmarshall - nasty memcpy replacement.
				//				Bmemcpy(w->wall.buffer, &ns->floor.buffer[nnwallnum - nsec->wallptr], sizeof(GLfloat) * 3);
				w->wall.buffer[0].position = ns->floor.buffer[nnwallnum - nsec->wallptr].position;
				// jmarshall end
			}
			// jmarshall - nasty memcpy replacement.
			//			Bmemcpy(&w->wall.buffer[1], &s->floor.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 3);
			//			Bmemcpy(&w->wall.buffer[2], &ns->floor.buffer[nwallnum - nsec->wallptr], sizeof(GLfloat) * 3);
			//			Bmemcpy(&w->wall.buffer[3], &ns->floor.buffer[nnwallnum - nsec->wallptr], sizeof(GLfloat) * 3);
			w->wall.buffer[1].position = s->floor.buffer[wal->point2 - sec->wallptr].position;
			w->wall.buffer[2].position = ns->floor.buffer[nwallnum - nsec->wallptr].position;
			w->wall.buffer[3].position = ns->floor.buffer[nnwallnum - nsec->wallptr].position;
			// jmarshall end


			if (wal->cstat & 2)
				refwall = nwallnum;
			else
				refwall = wallnum;

			curpicnum = (wal->cstat & 2) ? nwallpicnum : wallpicnum;
			curpal = wall[refwall].pal;
			curshade = wall[refwall].shade;
			curxpanning = wall[refwall].xpanning;
			curypanning = wall[refwall].ypanning;

			//	w->wall.bucket = polymer_getbuildmaterial(&w->wall.material, curpicnum, curpal, curshade, sec->visibility, DAMETH_WALL);

			if (!(wall[refwall].cstat & 4))
				yref = nsec->floorz;
			else
				yref = sec->ceilingz;

			if (curypanning)
				// under
				ypancoef = calc_ypancoef(curypanning, curpicnum, !(wall[refwall].cstat & 4));
			else
				ypancoef = 0;

			i = 0;
			while (i < 4)
			{
				if ((i == 0) || (i == 3))
					dist = (float)xref;
				else
					dist = (float)(xref == 0);

				w->wall.buffer[i].uv.x = ((dist * 8.0f * wal->xrepeat) + curxpanning) / (float)(tilesiz[curpicnum].x);
				w->wall.buffer[i].uv.y = (-(float)(yref + (w->wall.buffer[i].position.y * 16)) / ((tilesiz[curpicnum].y * 2048.0f) / (float)(wal->yrepeat))) + ypancoef;

				if ((!(wal->cstat & 2) && (wal->cstat & 256)) ||
					((wal->cstat & 2) && (wall[nwallnum].cstat & 256)))
					w->wall.buffer[i].uv.y = -w->wall.buffer[i].uv.y;

				i++;
			}

			if (underwall)
				w->underover |= 1;
			// jmarshall - nasty memcpy replacement.
			//			Bmemcpy(w->mask.buffer, &w->wall.buffer[3], sizeof(GLfloat) * 5);
			//			Bmemcpy(&w->mask.buffer[1], &w->wall.buffer[2], sizeof(GLfloat) * 5);
			w->mask.buffer[0] = w->wall.buffer[3];
			w->mask.buffer[1] = w->wall.buffer[2];
			// jmarshall end
		}
		else
		{
			// jmarshall - nasty memcpy replacement.
			//			Bmemcpy(w->mask.buffer, &s->floor.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 5);
			//			Bmemcpy(&w->mask.buffer[1], &s->floor.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 5);
			w->mask.buffer[0] = s->floor.buffer[wallnum - sec->wallptr];
			w->mask.buffer[1] = s->floor.buffer[wal->point2 - sec->wallptr];
			// jmarshall end
		}

		if ((s->ceil.buffer[wallnum - sec->wallptr].position.y > ns->ceil.buffer[nnwallnum - nsec->wallptr].position.y) ||
			(s->ceil.buffer[wal->point2 - sec->wallptr].position.y > ns->ceil.buffer[nwallnum - nsec->wallptr].position.y))
			overwall = 1;

		if ((overwall) || (wal->cstat & 16) || (wal->cstat & 32))
		{
			if (w->over.buffer == NULL) {
				w->over.buffer = new Build3DVertex[4]; // (_prvert *)Xmalloc(4 * sizeof(_prvert));
				w->over.vertcount = 4;
			}
			// jmarshall - nasty memcpy replacement.
			//			Bmemcpy(w->over.buffer, &ns->ceil.buffer[nnwallnum - nsec->wallptr], sizeof(GLfloat) * 3);
			//			Bmemcpy(&w->over.buffer[1], &ns->ceil.buffer[nwallnum - nsec->wallptr], sizeof(GLfloat) * 3);
			w->over.buffer[0].position = ns->ceil.buffer[nnwallnum - nsec->wallptr].position;
			w->over.buffer[1].position = ns->ceil.buffer[nwallnum - nsec->wallptr].position;
			// jmarshall end
			if (s->ceil.buffer[wal->point2 - sec->wallptr].position.y > ns->ceil.buffer[nwallnum - nsec->wallptr].position.y)
			{
				// jmarshall - nasty memcpy replacement.
				//				Bmemcpy(&w->over.buffer[2], &s->ceil.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 3);
				w->over.buffer[2].position = s->ceil.buffer[wal->point2 - sec->wallptr].position;
				// jmarshall end
			}
			else
			{
				// jmarshall - nasty memcpy replacement.
				//				Bmemcpy(&w->over.buffer[2], &ns->ceil.buffer[nwallnum - nsec->wallptr], sizeof(GLfloat) * 3);
				w->over.buffer[2].position = ns->ceil.buffer[nwallnum - nsec->wallptr].position;
				// jmarshall end
			}

			// jmarshall - nasty memcpy replacement.
			//			Bmemcpy(&w->over.buffer[3], &s->ceil.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 3);
			w->over.buffer[3].position = s->ceil.buffer[wallnum - sec->wallptr].position;
			// jmarshall end

			if ((wal->cstat & 16) || (wal->overpicnum == 0))
				curpicnum = wallpicnum;
			else
				curpicnum = wallpicnum;

			//w->over.bucket = polymer_getbuildmaterial(&w->over.material, curpicnum, wal->pal, wal->shade, sec->visibility, DAMETH_WALL);

			if ((wal->cstat & 16) || (wal->cstat & 32))
			{
				// mask
				//w->mask.bucket = polymer_getbuildmaterial(&w->mask.material, walloverpicnum, wal->pal, wal->shade, sec->visibility, DAMETH_WALL);

				if (wal->cstat & 128)
				{
					if (wal->cstat & 512)
						w->mask.diffusemodulation[3] = 0x55;
					else
						w->mask.diffusemodulation[3] = 0xAA;
				}
			}

			if (wal->cstat & 4)
				yref = sec->ceilingz;
			else
				yref = nsec->ceilingz;

			if (wal->ypanning)
				// over
				ypancoef = calc_ypancoef(wal->ypanning, curpicnum, wal->cstat & 4);
			else
				ypancoef = 0;

			i = 0;
			while (i < 4)
			{
				if ((i == 0) || (i == 3))
					dist = (float)xref;
				else
					dist = (float)(xref == 0);

				w->over.buffer[i].uv.x = ((dist * 8.0f * wal->xrepeat) + wal->xpanning) / (float)(tilesiz[curpicnum].x);
				w->over.buffer[i].uv.y = (-(float)(yref + (w->over.buffer[i].position.y * 16)) / ((tilesiz[curpicnum].y * 2048.0f) / (float)(wal->yrepeat))) + ypancoef;

				if (wal->cstat & 256) w->over.buffer[i].uv.y = -w->over.buffer[i].uv.y;

				i++;
			}

			if (overwall)
				w->underover |= 2;
			// jmarshall - nasty memcpy replacement.
			//			Bmemcpy(&w->mask.buffer[2], &w->over.buffer[1], sizeof(GLfloat) * 5);
			//			Bmemcpy(&w->mask.buffer[3], &w->over.buffer[0], sizeof(GLfloat) * 5);
			w->mask.buffer[2] = w->over.buffer[1];
			w->mask.buffer[3] = w->over.buffer[0];
			// jmarshall end

			if ((wal->cstat & 16) || (wal->cstat & 32))
			{
				const int botSwap = (wal->cstat & 4);

				if (wal->cstat & 32)
				{
					// 1-sided wall
					if (nsec)
						yref = botSwap ? sec->ceilingz : nsec->ceilingz;
					else
						yref = botSwap ? sec->floorz : sec->ceilingz;
				}
				else
				{
					// masked wall
					if (botSwap)
						yref = min(sec->floorz, nsec->floorz);
					else
						yref = max(sec->ceilingz, nsec->ceilingz);
				}

				curpicnum = walloverpicnum;

				if (wal->ypanning)
					// mask / 1-way
					ypancoef = calc_ypancoef(wal->ypanning, curpicnum, 0);
				else
					ypancoef = 0;

				i = 0;
				while (i < 4)
				{
					if ((i == 0) || (i == 3))
						dist = (float)xref;
					else
						dist = (float)(xref == 0);

					w->mask.buffer[i].uv.x = ((dist * 8.0f * wal->xrepeat) + wal->xpanning) / (float)(tilesiz[curpicnum].x);
					w->mask.buffer[i].uv.y = (-(float)(yref + (w->mask.buffer[i].position.y * 16)) / ((tilesiz[curpicnum].y * 2048.0f) / (float)(wal->yrepeat))) + ypancoef;

					if (wal->cstat & 256) w->mask.buffer[i].uv.y = -w->mask.buffer[i].uv.y;

					i++;
				}
			}
		}
		else
		{
			// jmarshall - nasty memcpy replacement.
			//			Bmemcpy(&w->mask.buffer[2], &s->ceil.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 5);
			//			Bmemcpy(&w->mask.buffer[3], &s->ceil.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 5);

			w->mask.buffer[2] = s->ceil.buffer[wal->point2 - sec->wallptr];
			w->mask.buffer[3] = s->ceil.buffer[wallnum - sec->wallptr];
			// jmarshall end
		}
	}

	// make sure shade color handling is correct below XXX
	if (wal->nextsector < 0)
	{
		// jmarshall this is stupid, but it works for now.
		Bmemcpy(w->mask.buffer, w->wall.buffer, sizeof(Build3DVertex) * 4);
	}

	Bmemcpy(w->bigportal, &s->floor.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 3);
	Bmemcpy(&w->bigportal[5], &s->floor.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 3);
	Bmemcpy(&w->bigportal[10], &s->ceil.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 3);
	Bmemcpy(&w->bigportal[15], &s->ceil.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 3);

	//Bmemcpy(&w->cap[0], &s->ceil.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 3);
	//Bmemcpy(&w->cap[3], &s->ceil.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 3);
	//Bmemcpy(&w->cap[6], &s->ceil.buffer[wal->point2 - sec->wallptr], sizeof(GLfloat) * 3);
	//Bmemcpy(&w->cap[9], &s->ceil.buffer[wallnum - sec->wallptr], sizeof(GLfloat) * 3);
	//w->cap[7] += 1048576; // this number is the result of 1048574 + 2
	//w->cap[10] += 1048576; // this one is arbitrary

	w->wall.sectorNum = sectorofwall(wallnum);

	if (w->wall.vbo_offset == -1)
	{
		w->wall.tileNum = wal->picnum;
		w->wall.vbo_offset = model->AddVertexesToBuffer(4, w->wall.buffer, w->wall.sectorNum);
		unsigned short indexes[6] = { 0, 1, 2, 3, 0, 2 };
		w->wall.indices = new unsigned short[6];
		memcpy(&w->wall.indices[0], &indexes[0], sizeof(unsigned short) * 6);
		w->wall.ibo_offset = model->AddIndexesToBuffer(6, indexes, w->wall.vbo_offset);
		w->wall.indicescount = 6;

		planelist.push_back(&w->wall);

		if (w->underover & 1)
		{
			computeplane(&w->wall);
			model->UpdateBuffer(w->wall.vbo_offset, 4, w->wall.buffer, w->wall.sectorNum, true);
		}

		//	newBoardPlanes.push_back(&w->wall);
	}
	else
	{
		w->wall.isDynamicPlane = true;
		w->wall.dynamic_vbo_offset = model->UpdateBuffer(w->wall.vbo_offset, 4, w->wall.buffer, w->wall.sectorNum);
		w->wall.vbo_offset = w->wall.dynamic_vbo_offset;
	}

	

	if (w->over.buffer)
	{
		w->over.sectorNum = sectorofwall(wallnum);
		if (w->over.vbo_offset == -1)
		{
			w->over.tileNum = wal->overpicnum; // is this right?
			if (w->over.tileNum == 0)
			{
				w->over.tileNum = wal->picnum;
			}
			w->over.vbo_offset = model->AddVertexesToBuffer(4, w->over.buffer, w->over.sectorNum);
			unsigned short indexes[6] = { 0, 1, 2, 3, 0, 2 };
			w->over.indices = new unsigned short[6];
			memcpy(&w->over.indices[0], &indexes[0], sizeof(unsigned short) * 6);
			w->over.ibo_offset = model->AddIndexesToBuffer(6, indexes, w->over.vbo_offset);
			w->over.indicescount = 6;

			if (w->underover & 2)
			{
				computeplane(&w->over);
				model->UpdateBuffer(w->over.vbo_offset, 4, w->over.buffer, w->over.sectorNum, true);
			}
			planelist.push_back(&w->over);
			//	newBoardPlanes.push_back(&w->over);
		}
		else
		{
			w->over.isDynamicPlane = true;
		//	model->UpdateBuffer(w->over.vbo_offset, 4, w->over.buffer);
			w->over.dynamic_vbo_offset = model->UpdateBuffer(w->over.vbo_offset, 4, w->over.buffer, w->over.sectorNum);
			w->over.vbo_offset = w->over.dynamic_vbo_offset;
		}

		
	}

	if (w->mask.buffer)
	{
		//if ((::wall[wallnum].cstat & 32) && (::wall[wallnum].nextsector >= 0))
		if ((wall[wallnum].cstat & 48) == 16)
		{
			w->mask.sectorNum = sectorofwall(wallnum);
			if (w->mask.vbo_offset == -1)
			{
				w->mask.tileNum = wal->overpicnum; // wrong?
				w->mask.vbo_offset = model->AddVertexesToBuffer(4, w->mask.buffer, w->mask.sectorNum);
				//newBoardPlanes.push_back(&w->mask);
				unsigned short indexes[6] = { 0, 1, 2, 3, 0, 2 };
				w->mask.indices = new unsigned short[6];
				memcpy(&w->mask.indices[0], &indexes[0], sizeof(unsigned short) * 6);
				w->mask.ibo_offset = model->AddIndexesToBuffer(6, indexes, w->mask.vbo_offset);
				w->mask.indicescount = 6;
				computeplane(&w->mask);
				model->UpdateBuffer(w->mask.vbo_offset, 4, w->mask.buffer, w->mask.sectorNum, true);
				planelist.push_back(&w->mask);
				w->mask.isMaskWall = true;
			}
			else
			{
				w->mask.isDynamicPlane = true;
				//model->UpdateBuffer(w->mask.vbo_offset, 4, w->mask.buffer);
				w->mask.dynamic_vbo_offset = model->UpdateBuffer(w->mask.vbo_offset, 4, w->mask.buffer, w->mask.sectorNum);
				w->mask.vbo_offset = w->mask.dynamic_vbo_offset;
			}
		}
		else
		{
			w->mask.tileNum = -1;
		}
	}

	//if ((pr_vbos > 0))
	//{
	//	if (pr_nullrender < 2)
	//	{
	//		const GLintptrARB thiswalloffset = prwalldataoffset + (prwalldatasize * wallnum);
	//		const GLintptrARB thisoveroffset = thiswalloffset + proneplanesize;
	//		const GLintptrARB thismaskoffset = thisoveroffset + proneplanesize;
	//		bglBindBufferARB(GL_ARRAY_BUFFER_ARB, prmapvbo);
	//		bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, thiswalloffset, proneplanesize, w->wall.buffer);
	//		bglBindBufferARB(GL_ARRAY_BUFFER_ARB, prmapvbo);
	//		if (w->over.buffer)
	//			bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, thisoveroffset, proneplanesize, w->over.buffer);
	//		bglBindBufferARB(GL_ARRAY_BUFFER_ARB, prmapvbo);
	//		bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, thismaskoffset, proneplanesize, w->mask.buffer);
	//		bglBindBufferARB(GL_ARRAY_BUFFER_ARB, w->stuffvbo);
	//		bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, 4 * sizeof(GLfloat) * 5, w->bigportal);
	//		//bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 4 * sizeof(GLfloat)* 5, 4 * sizeof(GLfloat)* 3, w->cap);
	//		bglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	//
	//		w->wall.mapvbo_vertoffset = thiswalloffset / sizeof(_prvert);
	//		w->over.mapvbo_vertoffset = thisoveroffset / sizeof(_prvert);
	//		w->mask.mapvbo_vertoffset = thismaskoffset / sizeof(_prvert);
	//	}
	//}
	//else
	//{
	//	w->wall.mapvbo_vertoffset = -1;
	//	w->over.mapvbo_vertoffset = -1;
	//	w->mask.mapvbo_vertoffset = -1;
	//}

	w->flags.empty = 0;
	w->flags.uptodate = 1;
	w->flags.invalidtex = 0;

	return true;
	//if (pr_verbosity >= 3) OSD_Printf("PR : Updated wall %i.\n", wallnum);
}

// SECTORS
/*
================
Build3DBoard::initsector
================
*/
bool Build3DBoard::initsector(int16_t sectnum)
{
	tsectortype      *sec;
	Build3DSector*      s;

	sec = (tsectortype *)&sector[sectnum];
	s = new Build3DSector();
	//s = (_prsector *)Xcalloc(1, sizeof(_prsector));

	s->verts = (double *)Xcalloc(sec->wallnum, sizeof(double) * 3);
	s->floor.buffer = new Build3DVertex[sec->wallnum];
	s->floor.vertcount = sec->wallnum;
	s->ceil.buffer = new Build3DVertex[sec->wallnum];
	s->ceil.vertcount = sec->wallnum;
	s->flags.empty = 1; // let updatesector know that everything needs to go

	prsectors[sectnum] = s;

	return true;
}

static inline void  polymer_normalize(float* vec)
{
	double norm;

	norm = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];

	norm = sqrt(norm);
	norm = 1.0 / norm;
	vec[0] *= norm;
	vec[1] *= norm;
	vec[2] *= norm;
}

/*
====================
computeplane
====================
*/
void computeplane(Build3DPlane* p)
{
	GLfloat         vec1[5], vec2[5], norm, r;// BxN[3], NxT[3], TxB[3];
	int32_t         i;
	Build3DVertex*    buffer;
	GLfloat*        plane;

	if (p->indices && (p->indicescount < 3))
		return; // corrupt sector (E3L4, I'm looking at you)

	buffer = p->buffer;
	plane = p->plane;

	i = 0;
	do
	{
		vec1[0] = buffer[(INDICE(1))].position.x - buffer[(INDICE(0))].position.x; //x1
		vec1[1] = buffer[(INDICE(1))].position.y - buffer[(INDICE(0))].position.y; //y1
		vec1[2] = buffer[(INDICE(1))].position.z - buffer[(INDICE(0))].position.z; //z1
		vec1[3] = buffer[(INDICE(1))].uv.x - buffer[(INDICE(0))].uv.x; //s1
		vec1[4] = buffer[(INDICE(1))].uv.y - buffer[(INDICE(0))].uv.y; //t1

		vec2[0] = buffer[(INDICE(2))].position.x - buffer[(INDICE(1))].position.x; //x2
		vec2[1] = buffer[(INDICE(2))].position.y - buffer[(INDICE(1))].position.y; //y2
		vec2[2] = buffer[(INDICE(2))].position.z - buffer[(INDICE(1))].position.z; //z2
		vec2[3] = buffer[(INDICE(2))].uv.x - buffer[(INDICE(1))].uv.x; //s2
		vec2[4] = buffer[(INDICE(2))].uv.y - buffer[(INDICE(1))].uv.y; //t2

		Build3dMath_CrossProduct(vec2, vec1, plane);

		norm = plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2];

		// hack to work around a precision issue with slopes
		if (norm >= 15000)
		{
			float tangent[3][3];
			double det;

			// normalize the normal/plane equation and calculate its plane norm
			norm = -sqrt(norm);
			norm = 1.0 / norm;
			plane[0] *= norm;
			plane[1] *= norm;
			plane[2] *= norm;
			plane[3] = -(plane[0] * buffer->position.x + plane[1] * buffer->position.y + plane[2] * buffer->position.z);

			buffer[(INDICE(0))].normal.x = plane[0];
			buffer[(INDICE(0))].normal.y = plane[1];
			buffer[(INDICE(0))].normal.z = plane[2];

			buffer[(INDICE(1))].normal.x = plane[0];
			buffer[(INDICE(1))].normal.y = plane[1];
			buffer[(INDICE(1))].normal.z = plane[2];

			buffer[(INDICE(2))].normal.x = plane[0];
			buffer[(INDICE(2))].normal.y = plane[1];
			buffer[(INDICE(2))].normal.z = plane[2];

			// calculate T and B
			r = 1.0 / (vec1[3] * vec2[4] - vec2[3] * vec1[4]);

			// tangent
			tangent[0][0] = (vec2[4] * vec1[0] - vec1[4] * vec2[0]) * r;
			tangent[0][1] = (vec2[4] * vec1[1] - vec1[4] * vec2[1]) * r;
			tangent[0][2] = (vec2[4] * vec1[2] - vec1[4] * vec2[2]) * r;

			polymer_normalize(&tangent[0][0]);

			// bitangent
			tangent[1][0] = (vec1[3] * vec2[0] - vec2[3] * vec1[0]) * r;
			tangent[1][1] = (vec1[3] * vec2[1] - vec2[3] * vec1[1]) * r;
			tangent[1][2] = (vec1[3] * vec2[2] - vec2[3] * vec1[2]) * r;

			polymer_normalize(&tangent[1][0]);

			// normal
			tangent[2][0] = plane[0];
			tangent[2][1] = plane[1];
			tangent[2][2] = plane[2];

			INVERT_3X3(p->tbn, det, tangent);
		}

		i += (p->indices) ? 3 : 1;
	} while ((p->indices && i < p->indicescount) ||
		(!p->indices && i < p->vertcount));

}


/*
================
Build3DBoard::updatesector
================
*/
bool Build3DBoard::updatesector(int16_t sectnum)
{
	Build3DSector*      s;
	tsectortype      *sec;
	walltype        *wal;
	int32_t         i, j;
	int32_t         ceilz, florz;
	int32_t         tex, tey, heidiff;
	float           secangcos, secangsin, scalecoef, xpancoef, ypancoef;
	int32_t         ang, needfloor, wallinvalidate;
	int16_t         curstat, curpicnum, floorpicnum, ceilingpicnum;
	char            curxpanning, curypanning;
	Build3DVertex*        curbuffer;

	s = prsectors[sectnum];
	sec = (tsectortype *)&sector[sectnum];

	secangcos = secangsin = 2;

	needfloor = wallinvalidate = 0;

	// geometry
	wal = &wall[sec->wallptr];
	i = 0;
	while (i < sec->wallnum)
	{
		if ((-wal->x != s->verts[(i * 3) + 2]))
		{
			float z = -(float)wal->x;
			s->verts[(i * 3) + 2] = z;
			s->floor.buffer[i].position.z = z;
			s->ceil.buffer[i].position.z = z;
			needfloor = wallinvalidate = 1;
		}
		if ((wal->y != s->verts[i * 3]))
		{
			float x = (float)wal->y;
			s->verts[i * 3] = x;
			s->floor.buffer[i].position.x = x;
			s->ceil.buffer[i].position.x = x;
			needfloor = wallinvalidate = 1;
		}

		i++;
		wal = &wall[sec->wallptr + i];
	}

	if ((s->flags.empty) ||
		needfloor ||
		(sec->floorz != s->floorz) ||
		(sec->ceilingz != s->ceilingz) ||
		(sec->floorheinum != s->floorheinum) ||
		(sec->ceilingheinum != s->ceilingheinum))
	{
		wallinvalidate = 1;

		wal = &wall[sec->wallptr];
		i = 0;
		while (i < sec->wallnum)
		{
			getzsofslope(sectnum, wal->x, wal->y, &ceilz, &florz);
			s->floor.buffer[i].position.y = -(float)(florz) / 16.0f;
			s->ceil.buffer[i].position.y = -(float)(ceilz) / 16.0f;

			i++;
			wal = &wall[sec->wallptr + i];
		}

		s->floorz = sec->floorz;
		s->ceilingz = sec->ceilingz;
		s->floorheinum = sec->floorheinum;
		s->ceilingheinum = sec->ceilingheinum;
	}
	else if (sec->visibility != s->visibility)
	{
		//wallinvalidate = 1; jmarshall: there is no reason visibility changes should cause a geo update...right?
		s->visibility = sec->visibility;
	}

	floorpicnum = sec->floorpicnum;
	DO_TILE_ANIM(floorpicnum, sectnum);
	ceilingpicnum = sec->ceilingpicnum;
	DO_TILE_ANIM(ceilingpicnum, sectnum);

	if ((!s->flags.empty) && (!needfloor) &&
		(floorpicnum == s->floorpicnum_anim) &&
		(ceilingpicnum == s->ceilingpicnum_anim) &&
		!Bmemcmp(&s->ceilingstat, &sec->ceilingstat, offsetof(sectortype, visibility) - offsetof(sectortype, ceilingstat)))
		goto attributes;

	wal = &wall[sec->wallptr];
	i = 0;
	while (i < sec->wallnum)
	{
		j = 2;
		curstat = sec->floorstat;
		curbuffer = s->floor.buffer;
		curpicnum = floorpicnum;
		curxpanning = sec->floorxpanning;
		curypanning = sec->floorypanning;

		while (j)
		{
			if (j == 1)
			{
				curstat = sec->ceilingstat;
				curbuffer = s->ceil.buffer;
				curpicnum = ceilingpicnum;
				curxpanning = sec->ceilingxpanning;
				curypanning = sec->ceilingypanning;
			}

			if (!waloff[curpicnum])
				loadtile(curpicnum);

			if (((sec->floorstat & 64) || (sec->ceilingstat & 64)) &&
				((secangcos == 2) && (secangsin == 2)))
			{
				ang = (getangle(wall[wal->point2].x - wal->x, wall[wal->point2].y - wal->y) + 512) & 2047;
				secangcos = (float)(sintable[(ang + 512) & 2047]) / 16383.0f;
				secangsin = (float)(sintable[ang & 2047]) / 16383.0f;
			}

			// relative texturing
			if (curstat & 64)
			{
				xpancoef = (float)(wal->x - wall[sec->wallptr].x);
				ypancoef = (float)(wall[sec->wallptr].y - wal->y);

				tex = (int32_t)(xpancoef * secangsin + ypancoef * secangcos);
				tey = (int32_t)(xpancoef * secangcos - ypancoef * secangsin);
			}
			else {
				tex = wal->x;
				tey = -wal->y;
			}

			if ((curstat & (2 + 64)) == (2 + 64))
			{
				heidiff = (int32_t)(curbuffer[i].position.y - curbuffer[0].position.y);
				// don't forget the sign, tey could be negative with concave sectors
				if (tey >= 0)
					tey = (int32_t)sqrt((double)((tey * tey) + (heidiff * heidiff)));
				else
					tey = -(int32_t)sqrt((double)((tey * tey) + (heidiff * heidiff)));
			}

			if (curstat & 4)
				swaplong(&tex, &tey);

			if (curstat & 16) tex = -tex;
			if (curstat & 32) tey = -tey;

			scalecoef = (curstat & 8) ? 8.0f : 16.0f;

			if (curxpanning)
			{
				xpancoef = (float)(pow2long[picsiz[curpicnum] & 15]);
				xpancoef *= (float)(curxpanning) / (256.0f * (float)(tilesiz[curpicnum].x));
			}
			else
				xpancoef = 0;

			if (curypanning)
			{
				ypancoef = (float)(pow2long[picsiz[curpicnum] >> 4]);
				ypancoef *= (float)(curypanning) / (256.0f * (float)(tilesiz[curpicnum].y));
			}
			else
				ypancoef = 0;

			curbuffer[i].uv.x = (((float)(tex) / (scalecoef * tilesiz[curpicnum].x)) + xpancoef);
			curbuffer[i].uv.y = (((float)(tey) / (scalecoef * tilesiz[curpicnum].y)) + ypancoef);

			j--;
		}
		i++;
		wal = &wall[sec->wallptr + i];
	}

	s->floorxpanning = sec->floorxpanning;
	s->ceilingxpanning = sec->ceilingxpanning;
	s->floorypanning = sec->floorypanning;
	s->ceilingypanning = sec->ceilingypanning;

	s->isInvalid = true;

	i = -1;

attributes:
	if (wallinvalidate)
	{
		if (s->floor.vbo_offset == -1)
		{
			s->floor.tileNum = sec->floorpicnum;
			s->floor.vbo_offset = model->AddVertexesToBuffer(sec->wallnum, s->floor.buffer, sectnum);
			planelist.push_back(&s->floor);
			//	newBoardPlanes.push_back(&s->floor);
		}
		else
		{
			s->floor.isDynamicPlane = true;
			//model->UpdateBuffer(s->floor.vbo_offset, sec->wallnum, s->floor.buffer);
			s->floor.dynamic_vbo_offset = model->UpdateBuffer(s->floor.vbo_offset, sec->wallnum, s->floor.buffer, sectnum);
			s->floor.vbo_offset = s->floor.dynamic_vbo_offset;
		}

		s->floor.sectorNum = sectnum;

		if (s->ceil.vbo_offset == -1)
		{
			s->ceil.tileNum = sec->ceilingpicnum;
			s->ceil.vbo_offset = model->AddVertexesToBuffer(sec->wallnum, s->ceil.buffer, sectnum);
			planelist.push_back(&s->ceil);
			//	newBoardPlanes.push_back(&s->ceil);
		}
		else
		{
			s->ceil.isDynamicPlane = true;
			//model->UpdateBuffer(s->ceil.vbo_offset, sec->wallnum, s->ceil.buffer);
			s->ceil.dynamic_vbo_offset = model->UpdateBuffer(s->ceil.vbo_offset, sec->wallnum, s->ceil.buffer, sectnum);
			s->ceil.vbo_offset = s->ceil.dynamic_vbo_offset;
		}

		s->ceil.sectorNum = sectnum;
	}

	//if ((pr_vbos > 0) && ((i == -1) || (wallinvalidate)))
	//{
	//	if (pr_vbos > 0)
	//	{
	//		if (pr_nullrender < 2)
	//		{
	//			/*bglBindBufferARB(GL_ARRAY_BUFFER_ARB, s->floor.vbo);
	//			bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sec->wallnum * sizeof(GLfloat)* 5, s->floor.buffer);
	//			bglBindBufferARB(GL_ARRAY_BUFFER_ARB, s->ceil.vbo);
	//			bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sec->wallnum * sizeof(GLfloat)* 5, s->ceil.buffer);
	//			*/
	//
	//			s->floor.mapvbo_vertoffset = sec->wallptr * 2;
	//			s->ceil.mapvbo_vertoffset = s->floor.mapvbo_vertoffset + sec->wallnum;
	//
	//			GLintptrARB sector_offset = s->floor.mapvbo_vertoffset * sizeof(_prvert);
	//			GLsizeiptrARB cur_sector_size = sec->wallnum * sizeof(_prvert);
	//			bglBindBufferARB(GL_ARRAY_BUFFER_ARB, prmapvbo);
	//			// floor
	//			bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sector_offset, cur_sector_size, s->floor.buffer);
	//			// ceiling
	//			bglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sector_offset + cur_sector_size, cur_sector_size, s->ceil.buffer);
	//			bglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	//		}
	//	}
	//	else
	//	{
	//		s->floor.mapvbo_vertoffset = -1;
	//		s->ceil.mapvbo_vertoffset = -1;
	//	}
	//}

	if ((!s->flags.empty) && (!s->flags.invalidtex) &&
		(floorpicnum == s->floorpicnum_anim) &&
		(ceilingpicnum == s->ceilingpicnum_anim) &&
		!Bmemcmp(&s->ceilingstat, &sec->ceilingstat, offsetof(sectortype, visibility) - offsetof(sectortype, ceilingstat)))
		goto finish;

	//s->floor.bucket = polymer_getbuildmaterial(&s->floor.material, floorpicnum, sec->floorpal, sec->floorshade, sec->visibility, 0);

	if (sec->floorstat & 256) {
		if (sec->floorstat & 128) {
			s->floor.diffusemodulation[3] = 0x55;
		}
		else {
			s->floor.diffusemodulation[3] = 0xAA;
		}
	}

	//s->ceil.bucket = polymer_getbuildmaterial(&s->ceil.material, ceilingpicnum, sec->ceilingpal, sec->ceilingshade, sec->visibility, 0);

	if (sec->ceilingstat & 256) {
		if (sec->ceilingstat & 128) {
			s->ceil.diffusemodulation[3] = 0x55;
		}
		else {
			s->ceil.diffusemodulation[3] = 0xAA;
		}
	}

	s->flags.invalidtex = 0;

	// copy ceilingstat through visibility members
	s->ceilingstat = sec->ceilingstat;
	s->floorstat = sec->floorstat;
	//Bmemcpy(&s->ceilingstat, &sec->ceilingstat, offsetof(sectortype, visibility) - offsetof(sectortype, ceilingstat));
	s->floorpicnum_anim = floorpicnum;
	s->ceilingpicnum_anim = ceilingpicnum;

finish:

	if (needfloor)
	{
		buildfloor(sectnum);

		if (s->floor.ibo_offset == -1)
		{
			s->floor.ibo_offset = model->AddIndexesToBuffer(s->indicescount, s->floor.indices, s->floor.vbo_offset);
		}

		if (s->ceil.ibo_offset == -1)
		{
			s->ceil.ibo_offset = model->AddIndexesToBuffer(s->indicescount, s->ceil.indices, s->ceil.vbo_offset);
		}

		//if ((pr_vbos > 0))
		//{
		//	if (pr_nullrender < 2)
		//	{
		//		if (s->oldindicescount < s->indicescount)
		//		{
		//			bglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, s->floor.ivbo);
		//			bglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, s->indicescount * sizeof(GLushort), NULL, mapvbousage);
		//			bglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, s->ceil.ivbo);
		//			bglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, s->indicescount * sizeof(GLushort), NULL, mapvbousage);
		//			s->oldindicescount = s->indicescount;
		//		}
		//		bglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, s->floor.ivbo);
		//		bglBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, s->indicescount * sizeof(GLushort), s->floor.indices);
		//		bglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, s->ceil.ivbo);
		//		bglBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, s->indicescount * sizeof(GLushort), s->ceil.indices);
		//		bglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		//	}
		//}
	}

	if (wallinvalidate)
	{
		s->invalidid++;
		//		polymer_invalidatesectorlights(sectnum);
		computeplane(&s->floor);
		computeplane(&s->ceil);

		// TODO: Performance!!!! !!This does ANOTHER memcpy!!!
		model->UpdateBuffer(s->floor.vbo_offset, sec->wallnum, s->floor.buffer, sectnum, true);
		model->UpdateBuffer(s->ceil.vbo_offset, sec->wallnum, s->ceil.buffer, sectnum, true);
	}

	s->flags.empty = 0;
	s->flags.uptodate = 1;

	s->boundingbox.Zero();

	for (int i = 0; i < s->ceil.vertcount; i++)
	{
		s->boundingbox.add(float3(s->ceil.buffer[i].position.x, s->ceil.buffer[i].position.y, s->ceil.buffer[i].position.z));
	}

	for (int i = 0; i < s->floor.vertcount; i++)
	{
		s->boundingbox.add(float3(s->floor.buffer[i].position.x, s->floor.buffer[i].position.y, s->floor.buffer[i].position.z));
	}

	return true;
}

void Build3DBoard::tesserror(int error)
{
	/* This callback is called by the tesselator whenever it raises an error.
	GLU_TESS_ERROR6 is the "no error"/"null" error spam in e1l1 and others. */

	//if (pr_verbosity >= 1 && error != GLU_TESS_ERROR6) OSD_Printf("PR : Tesselation error number %i reported : %s.\n", error, bgluErrorString(errno));
}

void Build3DBoard::tessedgeflag(int error)
{
	// Passing an edgeflag callback forces the tesselator to output a triangle list
	//	UNREFERENCED_PARAMETER(error);

}

void Build3DBoard::tessvertex(void* vertex, void* sector)
{
	Build3DSector*      s;

	s = (Build3DSector*)sector;

	if (s->curindice >= s->indicescount)
	{
		//if (pr_verbosity >= 2) OSD_Printf("PR : Indice overflow, extending the indices list... !\n");
		s->indicescount++;
		s->floor.indices = (unsigned short *)Xrealloc(s->floor.indices, s->indicescount * sizeof(unsigned short));
		s->ceil.indices = (unsigned short *)Xrealloc(s->ceil.indices, s->indicescount * sizeof(unsigned short));
	}
	s->ceil.indices[s->curindice] = (intptr_t)vertex;
	s->curindice++;
}

bool  Build3DBoard::buildfloor(int16_t sectnum)
{
	// This function tesselates the floor/ceiling of a sector and stores the triangles in a display list.
	Build3DSector*      s;
	tsectortype     *sec;
	intptr_t        i;

	//	if (pr_verbosity >= 2) OSD_Printf("PR : Tesselating floor of sector %i...\n", sectnum);

	s = prsectors[sectnum];
	sec = (tsectortype *)&sector[sectnum];

	if (s == NULL)
	{
		return false;
	}

	if (s->floor.indices == NULL)
	{
		s->indicescount = (max(3, sec->wallnum) - 2) * 3;
		s->floor.indices = (GLushort *)Xcalloc(s->indicescount, sizeof(GLushort));
		s->ceil.indices = (GLushort *)Xcalloc(s->indicescount, sizeof(GLushort));
	}

	s->curindice = 0;

	gluTessCallbackUWP(prtess, GLU_TESS_VERTEX_DATA, (void(*)(void))tessvertex);
	gluTessCallbackUWP(prtess, GLU_TESS_EDGE_FLAG, (void(*)(void))tessedgeflag);
	gluTessCallbackUWP(prtess, GLU_TESS_ERROR, (void(*)(void))tesserror);

	gluTessProperty(prtess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);

	gluTessBeginPolygon(prtess, s);
	gluTessBeginContour(prtess);

	i = 0;
	while (i < sec->wallnum)
	{
		gluTessVertex(prtess, s->verts + (3 * i), (void *)i);
		if ((i != (sec->wallnum - 1)) && ((sec->wallptr + i) > wall[sec->wallptr + i].point2))
		{
			gluTessEndContour(prtess);
			gluTessBeginContour(prtess);
		}
		i++;
	}
	gluTessEndContour(prtess);
	gluTessEndPolygon(prtess);

	i = 0;
	while (i < s->indicescount)
	{
		s->floor.indices[s->indicescount - i - 1] = s->ceil.indices[i];

		i++;
	}
	s->floor.indicescount = s->ceil.indicescount = s->indicescount;

	//	if (pr_verbosity >= 2) OSD_Printf("PR : Tesselated floor of sector %i.\n", sectnum);

	return true;
}

extern char textfont[2048], smalltextfont[2048];

/*
================
Build3D::printext256
================
*/
int32_t Build3D::printext256(int32_t xpos, int32_t ypos, int16_t col, int16_t backcol, const char *name, char fontsize)
{
	static PolymerNGMaterial *fontMaterial = NULL;
	int const arbackcol = (unsigned)backcol < 256 ? backcol : 0;

	// FIXME?
	if (col < 0)
		col = 0;

	palette_t p, b;

	bricolor(&p, col);
	bricolor(&b, arbackcol);

	if (fontMaterial == NULL)
	{
		fontMaterial = polymerNG.AllocFontImage(smalltextfont, textfont);
	}

	vec2f_t const tc = { fontsize ? (4.f / 256.f) : (8.f / 256.f),
		fontsize ? (6.f / 128.f) : (8.f / 128.f) };

	for (int c = 0; name[c]; ++c)
	{
		BuildRenderCommand command;
		BuildRenderThreadTaskRotateSprite			&taskRotateSprite = command.taskRotateSprite;

		command.taskId = BUILDRENDER_TASK_ROTATESPRITE;
		taskRotateSprite.isFontImage = true;
		taskRotateSprite.is2D = true;
		taskRotateSprite.spriteColor = float4(p.r, p.g, p.b, 255);
		taskRotateSprite.renderMaterialHandle = fontMaterial;
		taskRotateSprite.useOrtho = true;
		taskRotateSprite.forceHQShader = true;

		if (name[c] == '^' && isdigit(name[c + 1]))
		{
			char smallbuf[8];
			int bi = 0;

			while (isdigit(name[c + 1]) && bi < 3)
			{
				smallbuf[bi++] = name[c + 1];
				c++;
			}

			smallbuf[bi++] = 0;

			if (col)
				col = Batol(smallbuf);

			if ((unsigned)col >= 256)
				col = 0;

			bricolor(&p, col);

			continue;
		}

		vec2f_t const t = { (float)(name[c] % 32) * (1.0f / 32.f),
			(float)((name[c] / 32) + (fontsize * 8)) * (1.0f / 16.f) };

		float z = 0; 
		
		{
			BuildVertex vert;
			vert.textureCoords0 = float3(t.x, t.y, 1.0f);
			vert.vertex = float4(xpos, ypos, z, 1.0f);
			taskRotateSprite.vertexes[0] = vert; 
		}

		{
			BuildVertex vert;
			vert.textureCoords0 = float3(t.x + tc.x, t.y, 1.0f);
			vert.vertex = float4(xpos + (8 >> fontsize), ypos, z, 1.0f);
			taskRotateSprite.vertexes[1] = vert; 
		}

		{
			BuildVertex vert;
			vert.textureCoords0 = float3(t.x + tc.x, t.y + tc.y, 1.0f);
			vert.vertex = float4(xpos + (8 >> fontsize), ypos + (fontsize ? 6 : 8), z, 1.0f);
			taskRotateSprite.vertexes[2] = vert; 
		}

		{
			BuildVertex vert;
			vert.textureCoords0 = float3(t.x, t.y + tc.y, 1.0f);
			vert.vertex = float4(xpos, ypos + (fontsize ? 6 : 8), z, 1.0f);
			taskRotateSprite.vertexes[3] = vert; 
		}

		xpos += (8 >> fontsize);

		renderer.AddRenderCommand(command);
	}


	return 0;
}

/*
================
Build3D::drawpoly
================
*/
void Build3D::drawpoly(BuildRenderThreadTaskRotateSprite	&taskRotateSprite, vec2f_t const * const dpxy, int32_t const n, int32_t method)
{
#ifdef YAX_ENABLE
	if (g_nodraw) return;
#endif

	// jmarshall POYLMER_NG 11-25-2015
	// INS:

	// 2D Stuff's should only get drawn here.
	taskRotateSprite.is2D = true;
	// jmarshall end

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

		for (int i = n - 2, j = n - 1, k = 0; k < n; i = j, j = k, k++)
			f += (dpxy[i].x - dpxy[k].x)*dpxy[j].y;

		if (f <= 0) return;
	}

	if (palookup[globalpal] == NULL)
		globalpal = 0;

	//Load texture (globalpicnum)
	setgotpic(globalpicnum);
	vec2_t tsiz = tilesiz[globalpicnum];

	if (!waloff[globalpicnum])
	{
		loadtile(globalpicnum);

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

	for (int i = 0; i < n; ++i)
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

	//	if (skyclamphack) method |= DAMETH_CLAMPED;
	//
	//	pthtyp *pth = our_texcache_fetch(method&(~3));
	//
	//	if (!pth)
	//	{
	//		return;
	//	}

	static int32_t fullbright_pass = 0;

	//	if (pth->flags & PTH_HASFULLBRIGHT && indrawroomsandmasks && r_fullbrights)
	//	{
	//		if (!fullbright_pass)
	//			fullbright_pass = 1;
	//		else if (fullbright_pass == 2)
	//			pth = pth->ofb;
	//	}

	// If we aren't rendmode 3, we're in Polymer, which means this code is
	// used for rotatesprite only. Polymer handles all the material stuff,
	// just submit the geometry and don't mess with textures.
	//if (getrendermode() == REND_POLYMOST)
	//{
	//	glBindTexture(GL_TEXTURE_2D, pth ? pth->glpic : 0);
	//
	//	if (drawpoly_srepeat)
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//	if (drawpoly_trepeat)
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//}
	//// jmarshall
	//else if (getrendermode() == REND_POLYMERNG)
	//{
	//	if (pth)
	//	{
	//		taskRotateSprite.texnum = pth->glpic;
	//	}
	//	else
	//	{
	//		taskRotateSprite.texnum = 0;
	//	}
	//
	//	taskRotateSprite.drawpoly_srepeat = drawpoly_srepeat;
	//	taskRotateSprite.drawpoly_trepeat = drawpoly_trepeat;
	//}
	// jmarshall end

	// texture scale by parkar request
	//taskRotateSprite.textureScale_X *= pth->hicr->scale.x;
	//taskRotateSprite.textureScale_Y *= pth->hicr->scale.y;


	vec2f_t hacksc = { 1.f, 1.f };

	//if (pth && (pth->flags & PTH_HIGHTILE))
	//{
	//	hacksc = pth->scale;
	//	tsiz = pth->siz;
	//}

	vec2_t tsiz2 = tsiz;

	// D3D12 hardware supports non power of two textures.
	//if (!glinfo.texnpot)
	//{
	//	for (tsiz2.x = 1; tsiz2.x < tsiz.x; tsiz2.x += tsiz2.x)
	//		; /* do nothing */
	//	for (tsiz2.y = 1; tsiz2.y < tsiz.y; tsiz2.y += tsiz2.y)
	//		; /* do nothing */
	//}

	//if ((!(method & 3)) && (!fullbright_pass))
	{
		// jmarshall POYLMER_NG 11-25-2015
		// WAS:
		//        glDisable(GL_BLEND);
		//        glDisable(GL_ALPHA_TEST);
		taskRotateSprite.enableBlend = false;
		taskRotateSprite.enableAlpha = false;
		// jmarshall
	}
	//else
	//{
	//	//float const al = waloff[globalpicnum] ? alphahackarray[globalpicnum] * (1.f / 255.f) ? alphahackarray[globalpicnum] * (1.f / 255.f) :
	//	//	(pth && pth->hicr && pth->hicr->alphacut >= 0.f ? pth->hicr->alphacut : 0.f) : 0.f;
	//	//// jmarshall POYLMER_NG 11-25-2015
	//	// WAS:
	//	//		glAlphaFunc(GL_GREATER, al);
	//	//		glEnable(GL_BLEND);
	//	//		glEnable(GL_ALPHA_TEST);
	//	taskRotateSprite.enableAlpha = true;
	//	taskRotateSprite.enableBlend = true;
	//	//taskRotateSprite.alphaBlendType = al;
	//	// jmarshall
	//}

	float pc[4];

	float clamped_shade = min(max(globalshade * 1.0, 0), 38);
	float modulation = ((38 - clamped_shade) / 38) * 1.29;
	pc[0] = pc[1] = pc[2] = modulation;

	// Hack hack hack!!!!
	if (globalpal == 1)
	{
		pc[0] *= 0.6f;
		pc[1] *= 0.6f;
		pc[2] *= 1.0f;
	}
	else if (globalpal == 2)
	{
		pc[0] *= 1.0f;
		pc[1] *= 0.6f;
		pc[2] *= 0.6f;
	}
	
	//pc[0] *= (float)hictinting[globalpal].r * (1.f / 255.f);
	//pc[1] *= (float)hictinting[globalpal].g * (1.f / 255.f);
	//pc[2] *= (float)hictinting[globalpal].b * (1.f / 255.f);

	// spriteext full alpha control
	pc[3] = 1.0f; // float_trans[method & 3] * (1.f - drawpoly_alpha);

				  //	if (pth)
	{
		// jmarshall - implement this
		// tinting
		//if (!(hictinting[globalpal].f & HICTINT_PRECOMPUTED))
		//{
		//	if (pth->flags & PTH_HIGHTILE)
		//	{
		//		if (pth->palnum != globalpal || (pth->effects & HICTINT_IN_MEMORY) || (hictinting[globalpal].f & HICTINT_APPLYOVERALTPAL))
		//			hictinting_apply(pc, globalpal);
		//	}
		//	else if (hictinting[globalpal].f & HICTINT_USEONART)
		//		hictinting_apply(pc, globalpal);
		//}
		//
		//// global tinting
		//if ((pth->flags & PTH_HIGHTILE) && have_basepal_tint())
		//	hictinting_apply(pc, MAXPALOOKUPS - 1);
	}

	taskRotateSprite.spriteColor = float4(pc[0], pc[1], pc[2], pc[3]);

	//Hack for walls&masked walls which use textures that are not a power of 2
#if 0	
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
		for (int i = 0; i < npoints; ++i)
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


			{
				assert(!(npoints >= 4));
				for (i = 0; i < nn; i++)
				{
					vec2f_t const o = { uu[i], vv[i] };

					float const dp = o.x*ngdx + o.y*ngdy + ngdo,
						up = o.x*ngux + o.y*nguy + nguo,
						vp = o.x*ngvx + o.y*ngvy + ngvo;

					float const r = 1.f / dp;

					BuildVertex vert;
					vert.textureCoords0.SetX((up * r - du0 + uoffs) * invtsiz2.x);
					vert.textureCoords0.SetY(vp * r * invtsiz2.y);

					vert.vertex.SetX((o.x - ghalfx) * r * grhalfxdown10x);
					vert.vertex.SetY((ghoriz - o.y) * r * grhalfxdown10);
					vert.vertex.SetZ(r * (1.f / 1024.f));

					taskRotateSprite.vertexes[i] = vert; // push_back(vert);
				}
			}
		}
	}
	else
	{
#endif
		vec2f_t const scale = { 1.f / tsiz2.x * hacksc.x, 1.f / tsiz2.y * hacksc.y };


		{
			//			taskRotateSprite.vertexes.reserve(npoints);
//			assert(!(npoints != 4));
			for (int i = 0; i < npoints; i++)
			{
				float const r = 1.f / dd[i];
				BuildVertex vert;

				float x = (px[i] - ghalfx) * r * grhalfxdown10x;
				float y = (ghoriz - py[i]) * r * grhalfxdown10;
				float z = r * (1.f / 1024.f);

				vert.textureCoords0 = float3(uu[i] * r * scale.x, vv[i] * r * scale.y, 1.0f);
				vert.vertex = float4(x, y, z, 1.0f);
				taskRotateSprite.vertexes[i] = vert; // .push_back(vert);
			}
		}
#if 0
	}
#endif

// jmarshall
// this is the only render call we use here.
	taskRotateSprite.renderMaterialHandle = materialManager.LoadMaterialForTile(globalpicnum);
// jmarshall end
}


/*
==========================
Build3D::CalculateFogForPlane
==========================
*/
void Build3D::CalculateFogForPlane(int32_t tile, int32_t shade, int32_t vis, int32_t pal, Build3DPlane *plane)
{
	#define FOGDISTCONST 600
	#define FULLVIS_BEGIN 2.9e30
	#define FULLVIS_END 3.0e30

	Build3DVector4 fogcol, fogtable[MAXPALOOKUPS];

	if (shade > 0 && getrendermode() == REND_POLYMOST && r_usetileshades == 1 &&
		!(globalflags & GLOBAL_NO_GL_TILESHADES) &&
		(!usemodels || md_tilehasmodel(tile, pal) < 0))
		shade >>= 1;

	for (int i = 0; i <= MAXPALOOKUPS - 1; i++)
	{
		fogtable[i].x = palookupfog[i].r * (1.f / 255.f);
		fogtable[i].y = palookupfog[i].g * (1.f / 255.f);
		fogtable[i].z = palookupfog[i].b * (1.f / 255.f);
		fogtable[i].w = 0;
	}

	fogcol = fogtable[pal];

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

	plane->fogColor[0] = fogcol.x;
	plane->fogColor[1] = fogcol.y;
	plane->fogColor[2] = fogcol.z;

	plane->fogDensity = fogresult;
	plane->fogStart = fogresult;
	plane->fogEnd = fogresult2;
}

/*
================
Build3D::dorotatesprite
================
*/
void Build3D::dorotatesprite(BuildRenderCommand &command, int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum, int8_t dashade, char dapalnum, int32_t dastat, uint8_t daalpha, int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2, int32_t uniqid)
{
	BuildRenderThreadTaskRotateSprite			&taskRotateSprite = command.taskRotateSprite;

	command.taskId = BUILDRENDER_TASK_ROTATESPRITE;

//	assert(picnum > 0);
	assert(picnum < MAXTILES);

	//	drawpoly_alpha = daalpha * (1.0f / 255.0f);
	taskRotateSprite.texnum = picnum;

	//glox1 = -1; //Force fullscreen (glox1=-1 forces it to restore)

	globvis = 0;

	int32_t const ogpicnum = globalpicnum;
	globalpicnum = picnum;
	int32_t const  ogshade = globalshade;
	globalshade = dashade;
	int32_t const  ogpal = globalpal;
	globalpal = (int32_t)((uint8_t)dapalnum);
	float const  oghalfx = ghalfx;
	ghalfx = fxdim * .5f;
	float const  ogrhalfxdown10 = grhalfxdown10;
	grhalfxdown10 = 1.f / (ghalfx * 1024.f);
	float const  ogrhalfxdown10x = grhalfxdown10x;
	grhalfxdown10x = grhalfxdown10;
	float const  oghoriz = ghoriz;
	ghoriz = fydim * .5f;
	int32_t const  ofoffset = frameoffset;
	frameoffset = frameplace;
	float const  ogchang = gchang;
	gchang = 1.f;
	float const  ogshang = gshang;
	gshang = 0.f;
	float const  ogctang = gctang;
	gctang = 1.f;
	float const  ogstang = gstang;
	gstang = 0.f;




	int32_t method = 4; //Use OpenGL clamping - dorotatesprite never repeats

	if (!(dastat & RS_NOMASK))
	{
		if (dastat & RS_TRANS1)
			method |= (dastat & RS_TRANS2) ? 3 : 2;
		else
			method |= 1;
	}

	//	drawpoly_alpha = daalpha * (1.0f / 255.0f);

	vec2_t const siz = tilesiz[globalpicnum];
	vec2_t ofs = { 0, 0 };

	if (!(dastat & RS_TOPLEFT))
	{
		ofs.x = picanm[globalpicnum].flags.xofs + (siz.x >> 1);
		ofs.y = picanm[globalpicnum].flags.yofs + (siz.y >> 1);
	}

	if (dastat & RS_YFLIP)
		ofs.y = siz.y - ofs.y;

	int32_t ourxyaspect, temp;
	dorotspr_handle_bit2(&sx, &sy, &z, dastat, cx1 + cx2, cy1 + cy2, &temp, &ourxyaspect);

	float d = (float)z * (1.0f / (65536.f * 16384.f));
	float const cosang = (float)sintable[(a + 512) & 2047] * d;
	float cosang2 = cosang;
	float const sinang = (float)sintable[a & 2047] * d;
	float sinang2 = sinang;

	if ((dastat & RS_AUTO) || (!(dastat & RS_NOCLIP)))  // Don't aspect unscaled perms
	{
		d = (float)ourxyaspect * (1.0f / 65536.f);
		cosang2 *= d;
		sinang2 *= d;
	}

	float const cx = (float)sx * (1.0f / 65536.f) - (float)ofs.x * cosang2 + (float)ofs.y * sinang2;
	float const cy = (float)sy * (1.0f / 65536.f) - (float)ofs.x * sinang - (float)ofs.y * cosang;

	vec2f_t pxy[8] = { { cx, cy },
	{ cx + (float)siz.x * cosang2, cy + (float)siz.x * sinang },
	{ 0, 0 },
	{ cx - (float)siz.y * sinang2, cy + (float)siz.y * cosang } };

	pxy[2].x = pxy[1].x + pxy[3].x - pxy[0].x;
	pxy[2].y = pxy[1].y + pxy[3].y - pxy[0].y;

	int32_t n = 4;

	xtex.d = 0; ytex.d = 0; otex.d = 1.f;
	//px[0]*gux + py[0]*guy + guo = 0
	//px[1]*gux + py[1]*guy + guo = xsiz-.0001
	//px[3]*gux + py[3]*guy + guo = 0
	d = 1.f / (pxy[0].x*(pxy[1].y - pxy[3].y) + pxy[1].x*(pxy[3].y - pxy[0].y) + pxy[3].x*(pxy[0].y - pxy[1].y));

	float const sxd = ((float)siz.x - .0001f)*d;

	xtex.u = (pxy[3].y - pxy[0].y)*sxd;
	ytex.u = (pxy[0].x - pxy[3].x)*sxd;
	otex.u = 0 - pxy[0].x*xtex.u - pxy[0].y*ytex.u;

	float const syd = ((float)siz.y - .0001f)*d;

	if (!(dastat & RS_YFLIP))
	{
		//px[0]*gvx + py[0]*gvy + gvo = 0
		//px[1]*gvx + py[1]*gvy + gvo = 0
		//px[3]*gvx + py[3]*gvy + gvo = ysiz-.0001
		xtex.v = (pxy[0].y - pxy[1].y)*syd;
		ytex.v = (pxy[1].x - pxy[0].x)*syd;
		otex.v = 0 - pxy[0].x*xtex.v - pxy[0].y*ytex.v;
	}
	else
	{
		//px[0]*gvx + py[0]*gvy + gvo = ysiz-.0001
		//px[1]*gvx + py[1]*gvy + gvo = ysiz-.0001
		//px[3]*gvx + py[3]*gvy + gvo = 0
		xtex.v = (pxy[1].y - pxy[0].y)*syd;
		ytex.v = (pxy[0].x - pxy[1].x)*syd;
		otex.v = (float)siz.y - .0001f - pxy[0].x*xtex.v - pxy[0].y*ytex.v;
	}

	cx2++; cy2++;
	//Clippoly4 (converted from int32_t to double)

	int32_t nn = z = 0;
	float px2[8], py2[8];

	do
	{
		int32_t zz = z + 1; if (zz == n) zz = 0;
		float const x1 = pxy[z].x, x2 = pxy[zz].x - x1;
		if (((float)cx1 <= x1) && (x1 <= (float)cx2)) { px2[nn] = x1; py2[nn] = pxy[z].y; nn++; }
		float fx = (float)(x2 <= 0 ? cx2 : cx1); d = fx - x1;
		if ((d < x2) != (d < 0)) { px2[nn] = fx; py2[nn] = (pxy[zz].y - pxy[z].y)*d / x2 + pxy[z].y; nn++; }
		fx = (float)(x2 <= 0 ? cx1 : cx2); d = fx - x1;
		if ((d < x2) != (d < 0)) { px2[nn] = fx; py2[nn] = (pxy[zz].y - pxy[z].y)*d / x2 + pxy[z].y; nn++; }
		z = zz;
	} while (z);

	if (nn >= 3)
	{
		n = z = 0;
		do
		{
			int32_t zz = z + 1; if (zz == nn) zz = 0;
			float const y1 = py2[z], y2 = py2[zz] - y1;
			if ((cy1 <= y1) && (y1 <= cy2)) { pxy[n].y = y1; pxy[n].x = px2[z]; n++; }
			float fy = (float)(y2 <= 0 ? cy2 : cy1); d = fy - y1;
			if ((d < y2) != (d < 0)) { pxy[n].y = fy; pxy[n].x = (px2[zz] - px2[z])*d / y2 + px2[z]; n++; }
			fy = (float)(y2 <= 0 ? cy1 : cy2); d = fy - y1;
			if ((d < y2) != (d < 0)) { pxy[n].y = fy; pxy[n].x = (px2[zz] - px2[z])*d / y2 + px2[z]; n++; }
			z = zz;
		} while (z);

		//if (getrendermode() != REND_POLYMERNG)
		//	glDisable(GL_FOG);

		pow2xsplit = 0; drawpoly(taskRotateSprite, pxy, n, method);

		//if (getrendermode() != REND_POLYMERNG)
		//	if (!nofog) glEnable(GL_FOG);
	}

#ifdef POLYMER
	if (getrendermode() == REND_POLYMER)
	{
		r_detailmapping = olddetailmapping;
		r_glowmapping = oldglowmapping;
		polymer_postrotatesprite();
		pr_normalmapping = oldnormalmapping;
	}
#endif

	//if (getrendermode() != REND_POLYMERNG)
	//{
	//	glPopMatrix();
	//	glMatrixMode(GL_PROJECTION);
	//	glPopMatrix();
	//}

	globalpicnum = ogpicnum;
	globalshade = ogshade;
	globalpal = ogpal;
	ghalfx = oghalfx;
	grhalfxdown10 = ogrhalfxdown10;
	grhalfxdown10x = ogrhalfxdown10x;
	ghoriz = oghoriz;
	frameoffset = ofoffset;
	gchang = ogchang;
	gshang = ogshang;
	gctang = ogctang;
	gstang = ogstang;
}


const bool Build3DWall::ShouldRenderWall(tsectortype *sector, walltype *mapwall) const
{
	return (underover & 1) && !((sector->floorstat & 1) && (mapwall->nextsector >= 0) && (sector[mapwall->nextsector].floorstat & 1)) && !(mapwall->cstat & 32);
}

const bool Build3DWall::ShouldRenderOverWall(tsectortype *sector, walltype *mapwall) const
{
	return (underover & 2) && !((sector->ceilingstat & 1) && (mapwall->nextsector >= 0) && (sector[mapwall->nextsector].ceilingstat & 1)) && !(mapwall->cstat & 32);
}
