#pragma once

#include <string>
#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"

namespace DukeNukem
{
	#define MAX_DEBUG_TEXT 10
	// Renders the current FPS value in the bottom right corner of the screen using Direct2D and DirectWrite.
	class SampleFpsTextRenderer
	{
	public:
		SampleFpsTextRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		void UpdateString(std::wstring &text, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> color);
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Resources related to text rendering.
	//	std::wstring                                    m_text;
		int												numTextActive;
		//DWRITE_TEXT_METRICS	                            m_textMetrics[MAX_DEBUG_TEXT];
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_textColors[MAX_DEBUG_TEXT];
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_greenBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_yellowBrush;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_redBrush;
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;
		Microsoft::WRL::ComPtr<IDWriteTextLayout3>      m_textLayout[MAX_DEBUG_TEXT];
		Microsoft::WRL::ComPtr<IDWriteTextFormat2>      m_textFormat;
	};
}