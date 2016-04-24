#include "pch.h"
#include "DukeNukemMain.h"
#include "Common\DirectXHelper.h"
#include "Common\GraphicsContext.h"
#include "Build/include/BuildEngineApp.h"

using namespace DukeNukem;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
DukeNukemMain::DukeNukemMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/

	app->Startup();
}

DukeNukemMain::~DukeNukemMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void DukeNukemMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void DukeNukemMain::Update() 
{
	app->Update(0);

	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		m_sceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});
}

// This is REALLY evil!!!!!
ID3D11DeviceContext3 *context;
ID3D11RenderTargetView * targets[1];
ID3D11DepthStencilView * viewportDepthStencilView;
void RHIAppToggleDepthTest(bool enableDepthTest)
{
	if (enableDepthTest)
	{
		context->OMSetRenderTargets(1, targets, viewportDepthStencilView);
	}
	else
	{
		context->OMSetRenderTargets(1, targets, NULL);
	}
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool DukeNukemMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	if (app->HasWork())
	{
		GraphicsContext fakeContext; // Not used.

		context = m_deviceResources->GetD3DDeviceContext();

		// Reset the viewport to target the whole screen.
		auto viewport = m_deviceResources->GetScreenViewport();
		context->RSSetViewports(1, &viewport);

		// Reset render targets to the screen.
		viewportDepthStencilView = m_deviceResources->GetDepthStencilView();
		targets[0] = m_deviceResources->GetBackBufferRenderTargetView();
		context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

		// Clear the back buffer and depth stencil view.
		context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::Black);
		context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		app->RenderScene();

		// Disable the depth buffer for the UI pass.
		context->OMSetRenderTargets(1, targets, NULL);
		app->RenderUI(fakeContext);
	}
	else
	{
		return false;
	}

	return true;
}

// Notifies renderers that device resources need to be released.
void DukeNukemMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void DukeNukemMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
