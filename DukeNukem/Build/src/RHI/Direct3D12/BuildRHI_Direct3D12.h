// BuildRHI_Direct3D12.h
//

#pragma once

#include <D3D12SDKLayers.h>
#include <DXGItype.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <DXProgrammableCapture.h>

#include "../BuildRHI.h"
#include "../../../rhi/D3D12/Core/PipelineState.h"
#include "../../../rhi/D3D12/Core/GraphicsCore.h"
#include "../../../rhi/D3D12/Core/BufferManager.h"
#include "../../../rhi/D3D12/Core/CommandContext.h"

//
// BuildRHIPipelineStateObjectDirect3D12
//
class BuildRHIPipelineStateObjectDirect3D12 : public BuildRHIPipelineStateObject
{
public:
	BuildRHIPipelineStateObjectDirect3D12();

	virtual void SetInputLayout(BuildRHIInputElementDesc *desc);

	virtual void SetDepthState(bool depthEnabled);
	virtual void SetBlendMode(bool blendEnabled, bool additiveBlend, bool multiplicativeBlend);
	virtual void SetRasterizeState(bool twoSided);

	virtual void SetVertexShader(const void* Binary, size_t Size);
	virtual void SetPixelShader(const void* Binary, size_t Size);
	virtual void SetGeometryShader(const void* Binary, size_t Size);
	virtual void SetHullShader(const void* Binary, size_t Size);
	virtual void SetDomainShader(const void* Binary, size_t Size);

	virtual void Finalize(int numSamplers);

	GraphicsPSO &GetPSO() { return pso; }
	RootSignature &GetRootSigniture() { return s_RootSigniture; }
private:
	bool hasVertexShader;
	bool hasPixelShader;
	GraphicsPSO pso;
	RootSignature s_RootSigniture;
};

//
// BuildRHIInputElementDescDirect3D12
//
class BuildRHIInputElementDescDirect3D12 : public BuildRHIInputElementDesc
{
public:
	D3D12_INPUT_ELEMENT_DESC vertElem[20];
};

//
// BuildRHIInputElementDescDirect3D12UI
//
class BuildRHIInputElementDescDirect3D12UI : public BuildRHIInputElementDescDirect3D12
{
public:
	BuildRHIInputElementDescDirect3D12UI();

	virtual int GetNumElements() { return 2; }
};

//
// BuildRHIInputElementDescDirect3D12UI
//
class BuildRHIInputElementDescDirect3D12World : public BuildRHIInputElementDescDirect3D12
{
public:
	BuildRHIInputElementDescDirect3D12World();

	virtual int GetNumElements() { return 5; }
};

class BuildRHIInfoDirect3D12
{
public:
	static void EnableDirect3D12Logs();
};

//
// BuildRHIDirect3D12Local
//
class BuildRHIDirect3D12Local
{
public:

private:
	LinearAllocator *varying_allocator[2]; // Data that changes per-frame
	LinearAllocator *unvarying_allocator; // Data that doesn't change until the next level(e.g. level and model data).

};

extern BuildRHIInfoDirect3D12 d3d12Logging;