// Direct3D11_RHI_OcclusionQuery
//

#include "BuildRHI_Direct3D11.h"

BuildRHIGPUOcclusionQuery *BuildRHI::AllocateOcclusionQuery()
{
	return new BuildRHIDirect3D11GPUOcclusionQuery();
}

BuildRHIDirect3D11GPUOcclusionQuery::BuildRHIDirect3D11GPUOcclusionQuery()
{
	D3D11_QUERY_DESC desc;
	desc.Query = D3D11_QUERY_OCCLUSION;
	desc.MiscFlags = 0;
	DX::RHIGetD3DDevice()->CreateQuery(&desc, &rhiQuery);
}

void BuildRHIDirect3D11GPUOcclusionQuery::Begin()
{
	DX::RHIGetD3DDeviceContext()->Begin(rhiQuery);
}

void BuildRHIDirect3D11GPUOcclusionQuery::End()
{
	DX::RHIGetD3DDeviceContext()->End(rhiQuery);
}

bool BuildRHIDirect3D11GPUOcclusionQuery::IsVisible()
{
	uint64_t ret;
	while (S_OK != DX::RHIGetD3DDeviceContext()->GetData(rhiQuery, &ret, sizeof(ret), 0));
	return ret != 0;
}