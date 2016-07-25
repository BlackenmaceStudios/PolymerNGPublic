// WinMain.cpp
//

#include <windows.h>
#include <d3d11_1.h>
#include <d2d1_2.h>

#include "../Build/include/BuildEngineApp.h"
#include "../Common/GraphicsContext.h"

#include "../platform/Windows/include/SDL2/SDL.h"
#include "../platform/Windows/include/SDL2/SDL_syswm.h"

/*
=============================================================

EDuke32 Windows Launcher

The Win64 launcher for EDuke32.

=============================================================
*/

#pragma comment(lib, "Ws2_32.lib")

#include "../Editor/editor_manager.h"

#ifdef _DEBUG
#define BUILD_TYPE "Debug"
#else
#define BUILD_TYPE "Release"
#endif

#define EDUKE32_CLASSNAME "Eduke32"
#define EDUKE32_WINDOWS_TITLE "Eduke32 with PolymerNG(" BUILD_TYPE " - For Testing Purposes Only) - " __DATE__ " " __TIME__

HWND g_hwnd = NULL;

float globalWindowWidth = 0;
float globalWindowHeight = 0;

extern "C" char keystatus[256];

namespace DX
{
	ID3D11Device *g_d3d11Device_0 = NULL;
	ID3D11Device1 *g_d3d11Device = NULL;
	ID3D11DeviceContext1 *g_d3D11Context = NULL;
	ID3D11DeviceContext  *g_d3D11Context_0 = NULL;
	IDXGISwapChain *g_swapChain;

	ID3D11DeviceContext1 *g_d3d11ContextOverride = NULL;

	int g_videoCardMemory;

	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilState* m_depthStencilStateNoWrite;

	// Direct3D rendering objects. Required for 3D.
	ID3D11RenderTargetView	*m_d3dRenderTargetView;
	ID3D11DepthStencilView	*m_d3dDepthStencilView;
	D3D11_VIEWPORT		m_screenViewport;

	ID3D11Texture2D	*m_depthStencilBuffer;

	D3D_FEATURE_LEVEL g_d3dCurrentFeatureLevel;

	void SetRHID3DDeviceContextOverride(ID3D11DeviceContext1 *g_override) { g_d3d11ContextOverride = g_override; }

	ID3D11Device1 *RHIGetD3DDevice() { return g_d3d11Device; }
	ID3D11DeviceContext1* RHIGetD3DDeviceContext() 
	{ 
		if (g_d3d11ContextOverride != NULL)
			return g_d3d11ContextOverride;

		return g_d3D11Context; 
	}

	ID2D1Factory1 *RHIGetD2D1DeviceFactory3() { return NULL; }
	ID2D1DeviceContext1 *RHIGet2D1DeviceContext2() { return NULL; }

	//
	// InitializeGraphicsDevice
	// TODO: Break up this mess!
	//
	bool InitializeGraphicsDevice()
	{
		HRESULT result;
		IDXGIFactory* factory;
		//IDXGIAdapter* adapter;
		IDXGIOutput* adapterOutput;
		unsigned int numModes, i, numerator, denominator, stringLength;
		DXGI_MODE_DESC* displayModeList;
		DXGI_ADAPTER_DESC adapterDesc;
		int error;
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ID3D11Texture2D* backBufferPtr;

		// Create a DirectX graphics interface factory.
		//result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
		//if (FAILED(result))
		//{
		//	return false;
		//}
		//
		//// Use the factory to create an adapter for the primary graphics interface (video card).
		//result = factory->EnumAdapters(0, &adapter);
		//if (FAILED(result))
		//{
		//	return false;
		//}
		//
		//// Enumerate the primary adapter output (monitor).
		//result = adapter->EnumOutputs(0, &adapterOutput);
		//if (FAILED(result))
		//{
		//	return false;
		//}
		//
		//// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
		//result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
		//if (FAILED(result))
		//{
		//	return false;
		//}
		//
		//// Create a list to hold all the possible display modes for this monitor/video card combination.
		//displayModeList = new DXGI_MODE_DESC[numModes];
		//if (!displayModeList)
		//{
		//	return false;
		//}
		//
		//// Now fill the display mode list structures.
		//result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
		//if (FAILED(result))
		//{
		//	return false;
		//}
		//
		//// Now go through all the display modes and find the one that matches the screen width and height.
		//// When a match is found store the numerator and denominator of the refresh rate for that monitor.
		//for (i = 0; i < numModes; i++)
		//{
		//	if (displayModeList[i].Width == (unsigned int)globalWindowWidth)
		//	{
		//		if (displayModeList[i].Height == (unsigned int)globalWindowHeight)
		//		{
		//			numerator = displayModeList[i].RefreshRate.Numerator;
		//			denominator = displayModeList[i].RefreshRate.Denominator;
		//		}
		//	}
		//}
		//
		//// Get the adapter (video card) description.
		//result = adapter->GetDesc(&adapterDesc);
		//if (FAILED(result))
		//{
		//	return false;
		//}
		//
		//// Store the dedicated video card memory in megabytes.
		//DX::g_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);


		// Initialize the swap chain description.
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		// Set to a single back buffer.
		swapChainDesc.BufferCount = 1;

		// Set the width and height of the back buffer.
		swapChainDesc.BufferDesc.Width = globalWindowWidth;
		swapChainDesc.BufferDesc.Height = globalWindowHeight;

		// Set regular 32-bit surface for the back buffer.
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;


		// Set the usage of the back buffer.
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		// Set the handle for the window to render to.
		swapChainDesc.OutputWindow = g_hwnd;

		// Turn multisampling off.
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		// Set to full screen or windowed mode.
		swapChainDesc.Windowed = true;

		// Set the scan line ordering and scaling to unspecified.
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		// Discard the back buffer contents after presenting.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		// Don't set the advanced flags.
		swapChainDesc.Flags = 0;


		// This flag adds support for surfaces with a different color channel ordering
		// than the API default. It is required for compatibility with Direct2D.
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		// This array defines the set of DirectX hardware feature levels this app will support.
		// Note the ordering should be preserved.
		// Don't forget to declare your application's minimum required feature level in its
		// description.  All applications are assumed to support 9.1 unless otherwise stated.
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};

		// Create the Direct3D 11 API device object and a corresponding context.

		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr,					// Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
			0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			creationFlags,				// Set debug and Direct2D compatibility flags.
			featureLevels,				// List of feature levels this app can support.
			ARRAYSIZE(featureLevels),	// Size of the list above.
			D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&swapChainDesc, &g_swapChain,
			&g_d3d11Device_0,					// Returns the Direct3D device created.
			&g_d3dCurrentFeatureLevel,			// Returns feature level of device created.
			&g_d3D11Context_0					// Returns the device immediate context.
		);

		if (FAILED(hr))
		{
			return false;
		}

		// create d3d11_Device as normal, then
		hr = g_d3d11Device_0->QueryInterface(__uuidof (ID3D11Device1), (void **)&g_d3d11Device);

		if (SUCCEEDED(hr))
		{
			(void)g_d3D11Context_0->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_d3D11Context));
		}
		else
		{
			return false;
		}

		// throw away the original device
		g_d3d11Device_0->Release();

		// Get the pointer to the back buffer.
		result = DX::g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
		if (FAILED(result))
		{
			return false;
		}

		// Create the render target view with the back buffer pointer.
		result = DX::g_d3d11Device->CreateRenderTargetView(backBufferPtr, NULL, &DX::m_d3dRenderTargetView);
		if (FAILED(result))
		{
			return false;
		}

		// Release pointer to the back buffer as we no longer need it.
		backBufferPtr->Release();
		backBufferPtr = 0;

		D3D11_TEXTURE2D_DESC depthBufferDesc;

		// Initialize the description of the depth buffer.
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
		depthBufferDesc.Width = globalWindowWidth;
		depthBufferDesc.Height = globalWindowHeight;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		// Create the texture for the depth buffer using the filled out description.
		result = DX::g_d3d11Device->CreateTexture2D(&depthBufferDesc, NULL, &DX::m_depthStencilBuffer);
		if (FAILED(result))
		{
			return false;
		}

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		D3D11_RASTERIZER_DESC rasterDesc;

		// Initialize the description of the stencil state.
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		// Set up the description of the stencil state.
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing.
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing.
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create the depth stencil state.
		result = DX::g_d3d11Device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
		if (FAILED(result))
		{
			return false;
		}

		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		result = DX::g_d3d11Device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStateNoWrite);
		if (FAILED(result))
		{
			return false;
		}

		// Set the depth stencil state.
		DX::g_d3D11Context->OMSetDepthStencilState(m_depthStencilState, 1);

		// Initailze the depth stencil view.
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		// Create the depth stencil view.
		result = DX::g_d3d11Device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &DX::m_d3dDepthStencilView);
		if (FAILED(result))
		{
			return false;
		}

		DX::m_screenViewport.TopLeftX = 0;
		DX::m_screenViewport.TopLeftY = 0;
		DX::m_screenViewport.Width = globalWindowWidth;
		DX::m_screenViewport.Height = globalWindowHeight;

		return true;
	}
}

// Right now we are defaulting to the desktop width and height!
// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}

// This is REALLY evil!!!!!
ID3D11RenderTargetView * targets[1];
ID3D11DepthStencilView * viewportDepthStencilView;

void RHIAppSwitchBackToDeviceRenderBuffers()
{
	DX::g_d3D11Context->OMSetRenderTargets(1, targets, viewportDepthStencilView);
}

void RHIAppToggleDepthWrite(bool enableDepthWrite)
{
	if (enableDepthWrite)
	{
		DX::g_d3D11Context->OMSetDepthStencilState(DX::m_depthStencilState, 1);
	}
	else
	{
		DX::g_d3D11Context->OMSetDepthStencilState(DX::m_depthStencilStateNoWrite, 1);
	}
}

void RHIAppToggleDepthTest(bool enableDepthTest)
{
	if (enableDepthTest)
	{
		DX::g_d3D11Context->OMSetRenderTargets(1, targets, viewportDepthStencilView);
	}
	else
	{
		DX::g_d3D11Context->OMSetRenderTargets(1, targets, NULL);
		DX::g_d3D11Context->RSSetViewports(1, &DX::m_screenViewport);
	}
}

void RHIApiSetupContext(ID3D11DeviceContext1 *context)
{
	// Reset the viewport to target the whole screen.
	context->RSSetViewports(1, &DX::m_screenViewport);

	// Reset render targets to the screen.
	viewportDepthStencilView = DX::m_d3dDepthStencilView;
	targets[0] = DX::m_d3dRenderTargetView;
	context->OMSetRenderTargets(1, targets, DX::m_d3dDepthStencilView);
}

void DrawFrame(GraphicsContext &fakeContext)
{
	// Reset the viewport to target the whole screen.
	DX::g_d3D11Context->RSSetViewports(1, &DX::m_screenViewport);

	// Reset render targets to the screen.
	viewportDepthStencilView = DX::m_d3dDepthStencilView;
	targets[0] = DX::m_d3dRenderTargetView;
	DX::g_d3D11Context->OMSetRenderTargets(1, targets, DX::m_d3dDepthStencilView);

	// Clear the back buffer and depth stencil view.
	float clearColor[4];
	clearColor[0] = clearColor[1] = clearColor[2] = 0.0f;
	clearColor[3] = 1.0f;
	DX::g_d3D11Context->ClearRenderTargetView(DX::m_d3dRenderTargetView, &clearColor[0]);
	DX::g_d3D11Context->ClearDepthStencilView(DX::m_d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	app->RenderScene();

	// Disable the depth buffer for the UI pass.
	DX::g_d3D11Context->OMSetRenderTargets(1, targets, NULL);
	app->RenderUI(fakeContext);

	DX::g_swapChain->Present(0, 0);
}

bool Sys_IsWindowActive()
{
	HWND activeWindow = GetForegroundWindow();
	return activeWindow == g_hwnd;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	GraphicsContext fakeContext;

	int window_width, window_height;

	SDL_Window *window;

	if (strstr(lpCmdLine, "-editor"))
		editorManager.SetIsEditorMode(true);
	else
		editorManager.SetIsEditorMode(false);

	SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

	GetDesktopResolution(window_width, window_height);
	window = SDL_CreateWindow(
		EDUKE32_WINDOWS_TITLE,              // window title
		0,								   // initial x position
		0,								   // initial y position
		window_width,                      // width, in pixels
		window_height,                     // height, in pixels
		SDL_WINDOW_SHOWN                  // flags - see below
	);

	if (window == NULL)
	{
		return 0;
	}

	globalWindowWidth = window_width;
	globalWindowHeight = window_height;

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);

	g_hwnd = wmInfo.info.win.window;

	// Initialize the graphics device
	if (!DX::InitializeGraphicsDevice())
	{
		MessageBox(g_hwnd, L"Your graphics card does not meet the requirements to run this build of PolymerNG!", L"Graphics Requirements NOT met!", 0);
		return 1;
	}

	// Start the game thread.
	app->Startup();

	int LastPeekMessage = 0;

	// Main message loop:
	while (true)
	{
		//Handle events on queue
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				break;
			}
		}

		app->Update(0);

		if (app->HasWork())
		{
			DrawFrame(fakeContext);
		}
	}

	return 0;
}