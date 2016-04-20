// Direct3D11_RHI_GpuBuffer.cpp
//

#include "BuildRHI_Direct3D11.h"

//
// BuildD3D11GPUBufferVertexBuffer::BuildD3D11GPUBufferVertexBuffer
//
BuildD3D11GPUBufferVertexBuffer::BuildD3D11GPUBufferVertexBuffer(int initialSize, int stride, void *initialData, bool isDynamicBuffer)
{
	InitBuffer(initialSize, stride, initialData, isDynamicBuffer);
}

//
// BuildD3D11GPUBufferVertexBuffer::InitBuffer
//
void BuildD3D11GPUBufferVertexBuffer::InitBuffer(int initialSize, int stride, void *initialData, bool isDynamicBuffer)
{
	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	if (!isDynamicBuffer)
	{
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0;
	}
	else
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

		if (initialData == NULL)
		{
			initialData = (void *)new byte[stride * initialSize];
		}
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	bufferDesc.ByteWidth = stride * initialSize;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.MiscFlags = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = initialData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	rhiStride = stride;

	// Create the vertex buffer.
	HRESULT hr = DX::RHIGetD3DDevice()->CreateBuffer(&bufferDesc, &InitData, &rhiVertexBufferHandle);
	if (FAILED(hr))
	{
//		OSD_Printf("Failed to create vertex buffer\n");
	}
}

//
// BuildD3D11GPUBufferVertexBuffer::UpdateBuffer
//
void BuildD3D11GPUBufferVertexBuffer::UpdateBuffer(void *data, int size, int offset)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	DX::RHIGetD3DDeviceContext()->Map(rhiVertexBufferHandle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		//	Update the vertex buffer here.
		memcpy(((byte *)mappedResource.pData) + offset, data, size);

	//	Reenable GPU access to the vertex buffer data.
	DX::RHIGetD3DDeviceContext()->Unmap(rhiVertexBufferHandle, 0);
}

void BuildD3D11GPUBufferVertexBuffer::Bind()
{
	UINT offset = 0;
	DX::RHIGetD3DDeviceContext()->IASetVertexBuffers(0, 1, &rhiVertexBufferHandle, (UINT *)&rhiStride, &offset);
	
}

/*
=========================================================

INDEX BUFFER

=========================================================
*/

//
// BuildD3D11GPUBufferIndexBuffer::BuildD3D11GPUBufferIndexBuffer
//
BuildD3D11GPUBufferIndexBuffer::BuildD3D11GPUBufferIndexBuffer(int initialSize, int stride, void *initialData, bool isDynamicBuffer)
{
	InitBuffer(initialSize, stride, initialData, isDynamicBuffer);
}

//
// BuildD3D11GPUBufferIndexBuffer::InitBuffer
//
void BuildD3D11GPUBufferIndexBuffer::InitBuffer(int initialSize, int stride, void *initialData, bool isDynamicBuffer)
{
	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	if (!isDynamicBuffer)
	{
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0;
	}
	else
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		if (initialData == NULL)
		{
			initialData = (void *)new byte[sizeof(unsigned short) * initialSize];
		}
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	bufferDesc.ByteWidth = sizeof( unsigned short ) * initialSize;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.MiscFlags = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = initialData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	rhiStride = stride;

	// Create the Index buffer.
	HRESULT hr = DX::RHIGetD3DDevice()->CreateBuffer(&bufferDesc, &InitData, &rhiIndexBufferHandle);
	if (FAILED(hr))
	{
		//		OSD_Printf("Failed to create Index buffer\n");
	}
}

//
// BuildD3D11GPUBufferIndexBuffer::UpdateBuffer
//
void BuildD3D11GPUBufferIndexBuffer::UpdateBuffer(void *data, int size, int offset)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	DX::RHIGetD3DDeviceContext()->Map(rhiIndexBufferHandle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	//	Update the Index buffer here.
	memcpy(((byte *)mappedResource.pData) + offset, data, size);

	//	Reenable GPU access to the Index buffer data.
	DX::RHIGetD3DDeviceContext()->Unmap(rhiIndexBufferHandle, 0);
}

void BuildD3D11GPUBufferIndexBuffer::Bind()
{
	UINT offset = 0;
	DX::RHIGetD3DDeviceContext()->IASetIndexBuffer(rhiIndexBufferHandle, DXGI_FORMAT_R16_UINT, offset);

}

/*
=========================================================

CONSTANT BUFFER

=========================================================
*/

//
// BuildD3D11GPUBufferConstantBuffer::BuildD3D11GPUBufferConstantBuffer
//
BuildD3D11GPUBufferConstantBuffer::BuildD3D11GPUBufferConstantBuffer(int initialSize, void *initialData)
{
	InitBuffer(initialSize, initialData);
}

//
// BuildD3D11GPUBufferConstantBuffer::InitBuffer
//
void BuildD3D11GPUBufferConstantBuffer::InitBuffer(int initialSize,  void *initialData)
{
	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	if (initialData == NULL)
	{
		initialData = (void *)new byte[initialSize];
	}
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.ByteWidth = initialSize;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	
	bufferDesc.MiscFlags = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = initialData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	// Create the Constant buffer.
	HRESULT hr = DX::RHIGetD3DDevice()->CreateBuffer(&bufferDesc, &InitData, &rhiConstantBufferHandle);
	if (FAILED(hr))
	{
		//		OSD_Printf("Failed to create Constant buffer\n");
	}
}

//
// BuildD3D11GPUBufferConstantBuffer::UpdateBuffer
//
void BuildD3D11GPUBufferConstantBuffer::UpdateBuffer(void *data, int size, int offset)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	DX::RHIGetD3DDeviceContext()->Map(rhiConstantBufferHandle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	//	Update the Constant buffer here.
	memcpy(((byte *)mappedResource.pData) + offset, data, size);

	//	Reenable GPU access to the Constant buffer data.
	DX::RHIGetD3DDeviceContext()->Unmap(rhiConstantBufferHandle, 0);
}

void BuildD3D11GPUBufferConstantBuffer::Bind()
{
	UINT offset = 0;
	DX::RHIGetD3DDeviceContext()->VSSetConstantBuffers(0, 1, &rhiConstantBufferHandle);
//	DX::RHIGetD3DDeviceContext()->GSSetConstantBuffers(0, 1, &rhiConstantBufferHandle);
}