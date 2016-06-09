#pragma once
#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning(disable : 4996)
#pragma warning(disable : 4101)

// EDuke32 Defines
#define ENGINE_USING_A_C
#define UNIVERSALWINDOWSAPP 1
#define HAVE_VORBIS 1

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include <concrt.h>

#undef small