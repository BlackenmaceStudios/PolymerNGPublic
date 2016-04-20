// DIrect3D12_RHI_PipelineStateObject.cpp
//

#include "BuildRHI_Direct3D12.h"

/*
======================
BuildRHI::CreatePipelineStateObject
======================
*/
BuildRHIPipelineStateObject *BuildRHI::CreatePipelineStateObject()
{
	return new BuildRHIPipelineStateObjectDirect3D12();
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::BuildRHIPipelineStateObjectDirect3D12
======================
*/
BuildRHIPipelineStateObjectDirect3D12::BuildRHIPipelineStateObjectDirect3D12()
{
	hasPixelShader = false;
	hasVertexShader = false;
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetDepthState
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetDepthState(bool depthEnabled)
{
	if(!depthEnabled)
		pso.SetDepthStencilState(Graphics::DepthStateDisabled);
	else
		pso.SetDepthStencilState(Graphics::DepthStateReadWrite);
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetBlendMode
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetBlendMode(bool blendEnabled, bool additiveBlend, bool multiplicativeBlend)
{
	if (!blendEnabled)
	{
		pso.SetBlendState(Graphics::BlendDisable);
		return;
	}
	else if (!additiveBlend && !multiplicativeBlend)
	{
		pso.SetBlendState(Graphics::BlendTraditional);
		return;
	}
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetRasterizeState
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetRasterizeState(bool twoSided)
{
	if (twoSided)
	{
		pso.SetRasterizerState(Graphics::RasterizerTwoSided);
		return;
	}

	pso.SetRasterizerState(Graphics::RasterizerDefault);
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetInputLayout
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetInputLayout(BuildRHIInputElementDesc *desc)
{
	BuildRHIInputElementDescDirect3D12 *descD3D12 = (BuildRHIInputElementDescDirect3D12 *)desc;
	pso.SetInputLayout(descD3D12->GetNumElements(), descD3D12->vertElem);
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetVertexShader
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetVertexShader(const void* Binary, size_t Size)
{
	pso.SetVertexShader(Binary, Size);
	hasVertexShader = true;
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetPixelShader
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetPixelShader(const void* Binary, size_t Size)
{
	pso.SetPixelShader(Binary, Size);
	hasPixelShader = true;
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetGeometryShader
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetGeometryShader(const void* Binary, size_t Size)
{
	pso.SetGeometryShader(Binary, Size);
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetGeometryShader
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetHullShader(const void* Binary, size_t Size)
{
	pso.SetHullShader(Binary, Size);
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::SetDomainShader
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::SetDomainShader(const void* Binary, size_t Size)
{
	pso.SetDomainShader(Binary, Size);
}

/*
======================
BuildRHIPipelineStateObjectDirect3D12::Finalize
======================
*/
void BuildRHIPipelineStateObjectDirect3D12::Finalize(int numSamplers)
{
	int numRootParams = 0;

	// Right now we are always rendering triangles.
	pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pso.SetRenderTargetFormats(1, &Graphics::g_OverlayBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);

	if (hasPixelShader)
		numRootParams++;

	if (hasVertexShader)
		numRootParams++;

	s_RootSigniture.Reset(numRootParams + numSamplers, numSamplers);

	for (int i = 0; i < numSamplers; i++)
	{
		s_RootSigniture.InitStaticSampler(i, Graphics::SamplerPointClampDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	}

	int currentRootParam = 0;
	if (hasVertexShader)
	{
		s_RootSigniture[currentRootParam++].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	}

	if (hasPixelShader)
	{
		s_RootSigniture[currentRootParam++].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	}

	for (int i = 0; i < numSamplers; i++)
	{
		s_RootSigniture[currentRootParam++].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, i, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	
	//s_RootSigniture[currentRootParam++].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	
	s_RootSigniture.Finalize(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	pso.SetRootSignature(s_RootSigniture);

	pso.Finalize();
}