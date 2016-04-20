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

}

BuildRHIMesh *BuildRHI::AllocateRHIMesh(int vertexSize, int numVertexes, void * initialData, bool isDynamic)
{
	BuildRHIDirect3DMesh *mesh = new BuildRHIDirect3DMesh();
	mesh->vertexbuffer = new BuildD3D11GPUBufferVertexBuffer(numVertexes, vertexSize, initialData, isDynamic);
	return mesh;
}

void BuildRHI::AllocateRHIMeshIndexes(BuildRHIMesh *mesh, int numIndexes, void * initialData, bool isDynamic)
{
	BuildRHIDirect3DMesh *mesh_internal = static_cast<BuildRHIDirect3DMesh *>(mesh);
	mesh_internal->indexBuffer = new BuildD3D11GPUBufferIndexBuffer(numIndexes, -1, initialData, isDynamic);
}

BuildRHIConstantBuffer *BuildRHI::AllocateRHIConstantBuffer(int size, void *initialData)
{
	BuildD3D11GPUBufferConstantBuffer *constantBuffer = new BuildD3D11GPUBufferConstantBuffer(size, initialData);
	return constantBuffer;
}