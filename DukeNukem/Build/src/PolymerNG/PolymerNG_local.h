// PolymerNG_local.h
//

#pragma once

#include "compat.h"
#include "build.h"
//#include "editor.h"
#include "pragmas.h"
#include "cache1d.h"
#include "a.h"
#include "osd.h"
#include "crc32.h"
#include "xxhash.h"
#include "lz4.h"
#include "colmatch.h"
#include "build3d.h"

#include "PolymerNG.h"
#include "PolymerNG_ImageManager.h"
#include "PolymerNG_RenderProgram.h"
#include "PolymerNG_RenderTarget.h"
#include "PolymerNG_Visibility.h"
#include "PolymerNG_Light.h"
#include "PolymerNG_Board.h"

//#include "ShaderBuild/ShaderBinary.h"
//#include "ShaderBuild/ShaderBuild.h"

void updatesectorbreadth(int32_t x, int32_t y, int16_t *sectnum);


//
// ScopedHeapMemory
//
class ScopedHeapMemory
{
public:
	ScopedHeapMemory(int length) { buffer = new char[length]; }

	char *GetBuffer() { return buffer; }

	~ScopedHeapMemory() { delete buffer; }
private:
	char *buffer;
};

//
// PolymerNGPrivate
//
class PolymerNGPrivate
{
public:
	PolymerNGBoard			*currentBoard;
};

extern float globalWindowWidth;
extern float globalWindowHeight;

extern PolymerNGPrivate polymerNGPrivate;