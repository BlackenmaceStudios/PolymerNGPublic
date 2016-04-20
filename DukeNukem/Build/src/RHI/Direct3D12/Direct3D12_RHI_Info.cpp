// Direct3D12_RHI_Info.cpp
//

#include "BuildRHI_Direct3D12.h"

BuildRHIInfoDirect3D12 d3d12Logging;

void BuildRHIInfoDirect3D12::EnableDirect3D12Logs()
{
	ID3D12InfoQueue*    m_infoQueue;
	HRESULT hr = Graphics::g_Device->QueryInterface(IID_ID3D12InfoQueue, (void**)&m_infoQueue);

	if (SUCCEEDED(hr))
	{
		m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
		m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
		m_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

		D3D12_INFO_QUEUE_FILTER filter;
		memset(&filter, 0, sizeof(filter));

		//D3D12_MESSAGE_CATEGORY catlist[] =
		//{
		//	D3D12_MESSAGE_CATEGORY_STATE_CREATION,
		//	D3D12_MESSAGE_CATEGORY_EXECUTION,
		//};
		//filter.DenyList.NumCategories = 2; // BX_COUNTOF(catlist);
		//filter.DenyList.pCategoryList = catlist;
		m_infoQueue->PushStorageFilter(&filter);

		m_infoQueue->Release();
	}

	ID3D12Debug *debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
}