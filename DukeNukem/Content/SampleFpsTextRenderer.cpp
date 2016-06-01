#include "pch.h"
#include "SampleFpsTextRenderer.h"

#include "Common/DirectXHelper.h"

using namespace DukeNukem;
using namespace Microsoft::WRL;

extern int gameExecTimeInMilliseconds;
extern int gpuExecTimeInMilliseconds;
extern int32_t numDisplayedRenderPlanes;

// Initializes D2D resources used for text rendering.
SampleFpsTextRenderer::SampleFpsTextRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) : 
	m_deviceResources(deviceResources)
{
	//ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));

	// Create device independent resources
	ComPtr<IDWriteTextFormat> textFormat;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_BLACK,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			32.0f,
			L"en-US",
			&textFormat
			)
		);

	DX::ThrowIfFailed(
		textFormat.As(&m_textFormat)
		);

	DX::ThrowIfFailed(
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
		);

	DX::ThrowIfFailed(
		m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
		);

	CreateDeviceDependentResources();
}

void SampleFpsTextRenderer::UpdateString(std::wstring &text, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> color)
{
	int texid = numTextActive;
	ComPtr<IDWriteTextLayout> textLayout;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextLayout(
			text.c_str(),
			(uint32)text.length(),
			m_textFormat.Get(),
			540.0f, // Max width of the input text.
			50.0f, // Max height of the input text.
			&textLayout
			)
		);

	DX::ThrowIfFailed(
		textLayout.As(&m_textLayout[texid])
		);

	//DX::ThrowIfFailed(
	//	m_textLayout->GetMetrics(&m_textMetrics[texid])
	//	);

	m_textColors[texid] = color;

	numTextActive++;
}

// Updates the text to be displayed.
void SampleFpsTextRenderer::Update(DX::StepTimer const& timer)
{
	std::wstring m_text = L"Total Frame: ";
	int totalFrameTimeMS = 0; 
	
	if (gameExecTimeInMilliseconds > gpuExecTimeInMilliseconds)
	{
		totalFrameTimeMS = gameExecTimeInMilliseconds;
	}
	else
	{
		totalFrameTimeMS = gpuExecTimeInMilliseconds;
	}
	m_text = m_text + std::to_wstring(totalFrameTimeMS) + L" ms";

	numTextActive = 0;

	if (totalFrameTimeMS < 16)
	{
		UpdateString(m_text, m_greenBrush);
	}
	else if (totalFrameTimeMS <= 30)
	{
		UpdateString(m_text, m_yellowBrush);
	}
	else
	{
		UpdateString(m_text, m_redBrush);
	}

	m_text = L"Game Thread:";
	m_text = m_text + std::to_wstring(gameExecTimeInMilliseconds) + L" ms";

	if (gameExecTimeInMilliseconds < 16)
	{
		UpdateString(m_text, m_greenBrush);
	}
	else if (gameExecTimeInMilliseconds <= 30)
	{
		UpdateString(m_text, m_yellowBrush);
	}
	else
	{
		UpdateString(m_text, m_redBrush);
	}

	m_text = L"GPU:";
	m_text = m_text + std::to_wstring(gpuExecTimeInMilliseconds) + L" ms";

	if (gpuExecTimeInMilliseconds < 16)
	{
		UpdateString(m_text, m_greenBrush);
	}
	else if (gpuExecTimeInMilliseconds <= 30)
	{
		UpdateString(m_text, m_yellowBrush);
	}
	else
	{
		UpdateString(m_text, m_redBrush);
	}

	m_text = L"Render Planes:";
	m_text = m_text + std::to_wstring(numDisplayedRenderPlanes);

	if (numDisplayedRenderPlanes < 500)
	{
		UpdateString(m_text, m_greenBrush);
	}
	else if (numDisplayedRenderPlanes <= 800)
	{
		UpdateString(m_text, m_yellowBrush);
	}
	else
	{
		UpdateString(m_text, m_redBrush);
	}

}

// Renders a frame to the screen.
void SampleFpsTextRenderer::Render()
{
	ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
	Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

	context->SaveDrawingState(m_stateBlock.Get());
	context->BeginDraw();

	for (int i = 0; i < numTextActive; i++)
	{
		// Position on the bottom right corner
		D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
			0,
			i * 25
			);

		context->SetTransform(screenTranslation * m_deviceResources->GetOrientationTransform2D());

		DX::ThrowIfFailed(
			m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
			);

		context->DrawTextLayout(
			D2D1::Point2F(0.f, 0.f),
			m_textLayout[i].Get(),
			m_textColors[i].Get()
			);
	}

	// Ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	// is lost. It will be handled during the next call to Present.
	HRESULT hr = context->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr);
	}

	context->RestoreDrawingState(m_stateBlock.Get());
}

void SampleFpsTextRenderer::CreateDeviceDependentResources()
{
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_greenBrush)
		);
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &m_yellowBrush)
		);
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_redBrush)
		);
}
void SampleFpsTextRenderer::ReleaseDeviceDependentResources()
{
	m_greenBrush.Reset();
	m_yellowBrush.Reset();
	m_redBrush.Reset();
}