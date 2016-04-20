// Direct3D12_RHI.cpp
//

#include "../BuildRHI.h"
#include "BuildRHI_Direct3D12.h"

BuildRHI rhi;
IDXGraphicsAnalysis *graphicsAnalysis;

void BuildRHI::SetImageForContext(class GraphicsContext& Context, int rootIndex, const BuildRHITexture *image)
{
	Context.SetDynamicDescriptors(rootIndex, 0, 1, &image->GetSRV());
}

void BuildRHI::SetPSOForContext(class GraphicsContext& Context, BuildRHIPipelineStateObject *pso)
{
	BuildRHIPipelineStateObjectDirect3D12 *psoD3D12 = (BuildRHIPipelineStateObjectDirect3D12 *)pso;
	Context.SetRootSignature(psoD3D12->GetRootSigniture());
	Context.SetPipelineState(psoD3D12->GetPSO());
	Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void BuildRHI::DrawUnoptimized2DQuad(class GraphicsContext& Context, BuildRHIUIVertex *vertexes)
{
	Context.SetDynamicVB(0, 4, sizeof(BuildRHIUIVertex), vertexes);
	Context.DrawInstanced(4, 1);
}

void BuildRHI::Init()
{
	d3d12Logging.EnableDirect3D12Logs();
}