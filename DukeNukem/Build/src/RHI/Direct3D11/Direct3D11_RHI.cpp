// Direct3D11_RHI.cpp
//

#include "BuildRHI_Direct3D11.h"

BuildRHIDirect3D11Private rhiPrivate;

void BuildRHI::Init()
{
	initprintf("Initializing RHI Gui Mesh\n");
	rhiPrivate.guiRHIMesh.vertexbuffer = new BuildD3D11GPUBufferVertexBuffer(10, sizeof(BuildRHIUIVertex), NULL, true);
	rhiPrivate.guiRHIMesh.indexBuffer = new BuildD3D11GPUBufferIndexBuffer(10, 0, NULL, true);

	initprintf("Resetting RHI Context\n");
	rhiPrivate.ResetContext();

	D3D11_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	if (FAILED(DX::RHIGetD3DDevice()->CreateSamplerState(&samplerDesc, &rhiPrivate.pointSampleState)))
	{
		initprintf("Failed to create point sample state\n");
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	if (FAILED(DX::RHIGetD3DDevice()->CreateSamplerState(&samplerDesc, &rhiPrivate.linearSampleState)))
	{
		initprintf("Failed to create point sample state\n");
	}

	{
		// We want to use back face culling.
		D3D11_RASTERIZER_DESC cullModeDesc;
		memset(&cullModeDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
		cullModeDesc.CullMode = D3D11_CULL_NONE; 
		cullModeDesc.FillMode = D3D11_FILL_SOLID;
		DX::RHIGetD3DDevice()->CreateRasterizerState(&cullModeDesc, &rhiPrivate.cullModeBuildState);

		// HACK HACK
		DX::RHIGetD3DDeviceContext()->RSSetState(rhiPrivate.cullModeBuildState);
	}

	{
		// We want to use back face culling.
		D3D11_RASTERIZER_DESC cullModeDesc;
		memset(&cullModeDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
		cullModeDesc.CullMode = D3D11_CULL_FRONT;
		cullModeDesc.FillMode = D3D11_FILL_SOLID;
		DX::RHIGetD3DDevice()->CreateRasterizerState(&cullModeDesc, &rhiPrivate.cullModeBuildFrontState);

	}

	{
		// We want to use back face culling.
		D3D11_RASTERIZER_DESC cullModeDesc;
		memset(&cullModeDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
		cullModeDesc.CullMode = D3D11_CULL_BACK;
		cullModeDesc.FillMode = D3D11_FILL_SOLID;
		DX::RHIGetD3DDevice()->CreateRasterizerState(&cullModeDesc, &rhiPrivate.cullModeBuildBackState);
	}


	{
		// Always have alpha blend enabled.
		D3D11_BLEND_DESC1 BlendState;
		
		ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC1));
		BlendState.RenderTarget[0].BlendEnable = TRUE;
		BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BlendState.RenderTarget[0].RenderTargetWriteMask = 0x0f;
		DX::RHIGetD3DDevice()->CreateBlendState1(&BlendState, &rhiPrivate.alphaBlendState);
		DX::RHIGetD3DDeviceContext()->OMSetBlendState(rhiPrivate.alphaBlendState, 0, 0xffffffff);
	}

	{
		// Additive blend
		D3D11_BLEND_DESC1 BlendState;

		ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC1));
		BlendState.RenderTarget[0].BlendEnable = TRUE;
		BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BlendState.RenderTarget[0].RenderTargetWriteMask = 0x0f;
		DX::RHIGetD3DDevice()->CreateBlendState1(&BlendState, &rhiPrivate.additiveBlendState);
		//DX::RHIGetD3DDeviceContext()->OMSetBlendState(rhiPrivate.alphaBlendState, 0, 0xffffffff);
	}

	// Create our deferred context.
	if (FAILED(DX::RHIGetD3DDevice()->CreateDeferredContext(0, &rhiPrivate.pDeferredContext_0)))
	{
		initprintf("Failed to create deferred render context!!\n");
	}
	else
	{
		(void)(rhiPrivate.pDeferredContext_0)->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&rhiPrivate.pDeferredContext));
	}
}

BuildRHIMesh *BuildRHI::AllocateRHIMesh(int vertexSize, int numVertexes, void * initialData, bool isDynamic)
{
	BuildRHIDirect3DMesh *mesh = new BuildRHIDirect3DMesh();
	mesh->vertexbuffer = new BuildD3D11GPUBufferVertexBuffer(numVertexes, vertexSize, initialData, isDynamic);
	return mesh;
}

void BuildRHI::UpdateRHIMesh(BuildRHIMesh *mesh, int startVertex, int vertexSize, int numVertexes, void *initialData)
{
	BuildRHIDirect3DMesh *mesh_internal = static_cast<BuildRHIDirect3DMesh *>(mesh);
	mesh_internal->vertexbuffer->UpdateBuffer(initialData, vertexSize * numVertexes, vertexSize * startVertex);
}

void BuildRHI::AllocateRHIMeshIndexes(BuildRHIMesh *mesh, int numIndexes, void * initialData, bool isDynamic)
{
	BuildRHIDirect3DMesh *mesh_internal = static_cast<BuildRHIDirect3DMesh *>(mesh);
	mesh_internal->indexBuffer = new BuildD3D11GPUBufferIndexBuffer(numIndexes, -1, initialData, isDynamic);
}

void BuildRHI::SetRHIMeshIndexBuffer(BuildRHIMesh *mesh, BuildRHIMesh *parentMesh)
{
	BuildRHIDirect3DMesh *mesh_internal = static_cast<BuildRHIDirect3DMesh *>(mesh);
	BuildRHIDirect3DMesh *parentMesh_internal = static_cast<BuildRHIDirect3DMesh *>(parentMesh);

	mesh_internal->indexBuffer = parentMesh_internal->indexBuffer;
}

BuildRHIConstantBuffer *BuildRHI::AllocateRHIConstantBuffer(int size, void *initialData)
{
	BuildD3D11GPUBufferConstantBuffer *constantBuffer = new BuildD3D11GPUBufferConstantBuffer(size, initialData);
	return constantBuffer;
}

void RHIAppToggleDepthTest(bool enableDepthTest);
void RHIAppToggleDepthWrite(bool enableDepthWrite);

void BuildRHI::SetDepthEnable(bool depthEnable)
{
	RHIAppToggleDepthTest(depthEnable);
}

void BuildRHI::SetDepthWriteEnable(bool depthWriteEnable)
{
	RHIAppToggleDepthWrite(depthWriteEnable);
}