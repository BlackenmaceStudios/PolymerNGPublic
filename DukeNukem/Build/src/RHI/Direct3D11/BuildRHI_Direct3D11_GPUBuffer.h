// BuildRHI_Direct3D11_GPUBuffer.h
//

#pragma once

//
// BuildD3D11GPUBufferVertexBuffer
//
class BuildD3D11GPUBufferVertexBuffer
{
public:
	BuildD3D11GPUBufferVertexBuffer(int initialSize, int stride, void *initialData, bool isDynamicBuffer);

	void UpdateBuffer(void *data, int size, int offset);

	void Bind();
private:
	void InitBuffer(int initialSize, int stride, void *initialData, bool isDynamicBuffer);

	int rhiStride;
	int vertexBufferSize;
	ID3D11Buffer*      rhiVertexBufferHandle;
};

//
// BuildD3D11GPUBufferIndexBuffer
//
class BuildD3D11GPUBufferIndexBuffer
{
public:
	BuildD3D11GPUBufferIndexBuffer(int initialSize, int stride, void *initialData, bool isDynamicBuffer);

	void UpdateBuffer(void *data, int size, int offset);

	void Bind();
private:
	void InitBuffer(int initialSize, int stride, void *initialData, bool isDynamicBuffer);

	int rhiStride;
	ID3D11Buffer*      rhiIndexBufferHandle;
};

//
// BuildD3D11GPUBufferConstantBuffer
//
class BuildD3D11GPUBufferConstantBuffer : public BuildRHIConstantBuffer
{
public:
	BuildD3D11GPUBufferConstantBuffer(int initialSize, void *initialData);

	virtual void UpdateBuffer(void *data, int size, int offset);

	void Bind(BuildShaderBindTarget target);
private:
	void InitBuffer(int initialSize, void *initialData);

	int rhiStride;
	ID3D11Buffer*      rhiConstantBufferHandle;
};