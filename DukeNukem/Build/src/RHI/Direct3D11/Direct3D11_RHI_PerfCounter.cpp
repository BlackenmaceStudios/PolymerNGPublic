// Direct3D11_RHI_Counter.cpp
//

#include "BuildRHI_Direct3D11.h"

BuildRHIGPUPerformanceCounter *BuildRHI::AllocatePerformanceCounter()
{
	return new BuildRHIDirect3D11GPUPerformanceCounter();
}

BuildRHIDirect3D11GPUPerformanceCounter::BuildRHIDirect3D11GPUPerformanceCounter()
{
	D3D11_QUERY_DESC desc;
	desc.Query = D3D11_QUERY_TIMESTAMP;
	desc.MiscFlags = 0;
	DX::RHIGetD3DDevice()->CreateQuery(&desc, &rhiQueryStart);
	DX::RHIGetD3DDevice()->CreateQuery(&desc, &rhiQueryEnd);

	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	DX::RHIGetD3DDevice()->CreateQuery(&desc, &rhiQueryDisjoint);
}

void BuildRHIDirect3D11GPUPerformanceCounter::Begin()
{
	DX::RHIGetD3DDeviceContext()->Begin(rhiQueryDisjoint);
	DX::RHIGetD3DDeviceContext()->End(rhiQueryStart);
}

void BuildRHIDirect3D11GPUPerformanceCounter::End()
{
	DX::RHIGetD3DDeviceContext()->End(rhiQueryEnd);
	DX::RHIGetD3DDeviceContext()->End(rhiQueryDisjoint);
}

UINT64 BuildRHIDirect3D11GPUPerformanceCounter::GetTime()
{
	UINT64 startTime, endTime;
	while (DX::RHIGetD3DDeviceContext()->GetData(rhiQueryDisjoint, NULL, 0, 0) == S_FALSE)
	{
		Sleep(1);       // Wait a bit, but give other threads a chance to run
	}

	// Check whether timestamps were disjoint during the last frame
	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT tsDisjoint;
	DX::RHIGetD3DDeviceContext()->GetData(rhiQueryDisjoint, &tsDisjoint, sizeof(tsDisjoint), 0);

	while (DX::RHIGetD3DDeviceContext()->GetData(rhiQueryStart, &startTime, sizeof(UINT64), 0) == S_FALSE)
	{
		Sleep(1);
	}
	while (DX::RHIGetD3DDeviceContext()->GetData(rhiQueryEnd, &endTime, sizeof(UINT64), 0) == S_FALSE)
	{
		Sleep(1);
	}
	return float(endTime - startTime) / float(tsDisjoint.Frequency) * 1000.0f;
}