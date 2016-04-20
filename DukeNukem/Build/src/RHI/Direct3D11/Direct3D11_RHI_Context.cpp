// Direct3D11_RHI_Context.cpp
//

#include "BuildRHI_Direct3D11.h"

void BuildRHIDirect3D11Private::ResetContext()
{
	for (int i = 0; i < RHIMAX_BOUNDTEXTURES; i++)
	{
		rhiPrivate.boundTextures[i] = NULL;
	}

	DX::RHIGetD3DDeviceContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void BuildRHI::SetImageForContext(int rootIndex, const BuildRHITexture *image)
{
	const BuildRHITextureDirect3D11 *rhiImage = static_cast<const BuildRHITextureDirect3D11 *>(image);
	DX::RHIGetD3DDeviceContext()->PSSetShaderResources(rootIndex, 1, &rhiImage->resourceView);
	DX::RHIGetD3DDeviceContext()->PSSetSamplers(rootIndex, 1, &rhiPrivate.pointSampleState);
	rhiPrivate.boundTextures[rootIndex] = image;
}

void BuildRHI::SetShader(BuildRHIShader *shader)
{
	rhiPrivate.currentShader = static_cast<BuildRHIDirect3D11Shader *>(shader);
}

void BuildRHIDirect3D11Private::SetPixelShader(ID3D11PixelShader *pixelShader)
{
	static ID3D11PixelShader *currentPixelShader = NULL;
	if (pixelShader != currentPixelShader)
	{
		DX::RHIGetD3DDeviceContext()->PSSetShader(pixelShader, NULL, 0);
		pixelShader = currentPixelShader;
	}
}

void BuildRHIDirect3D11Private::SetGeomtryShader(ID3D11GeometryShader *geometryShader)
{
	static ID3D11GeometryShader *currentGeomtryShader = NULL;
	if (geometryShader != currentGeomtryShader)
	{
		DX::RHIGetD3DDeviceContext()->GSSetShader(geometryShader, NULL, 0);
		currentGeomtryShader = currentGeomtryShader;
	}
}

void BuildRHIDirect3D11Private::SetVertexShader(ID3D11VertexShader *vertexShader)
{
	static ID3D11VertexShader *currentVertexShader = NULL;
	if (vertexShader != currentVertexShader)
	{
		DX::RHIGetD3DDeviceContext()->VSSetShader(vertexShader, NULL, 0);
		vertexShader = currentVertexShader;
	}
}

void BuildRHIDirect3D11Private::SetInputLayout(ID3D11InputLayout *layout)
{
	static ID3D11InputLayout *currentLayout = NULL;

	if (layout != currentLayout)
	{
		DX::RHIGetD3DDeviceContext()->IASetInputLayout(layout);
		currentLayout = layout;
	}
}

void BuildRHI::SetConstantBuffer(int index, BuildRHIConstantBuffer *constantBuffer)
{
	static BuildRHIConstantBuffer *currentConstantBuffer = NULL;
	if (currentConstantBuffer != constantBuffer)
	{
		static_cast<BuildD3D11GPUBufferConstantBuffer *>(constantBuffer)->Bind();
		constantBuffer = currentConstantBuffer;
	}
}

void BuildRHI::DrawUnoptimizedQuad(BuildRHIShader *shader, BuildRHIMesh *mesh, int startVertex, int numVertexes)
{
	BuildRHIDirect3DMesh *mesh_internal = static_cast<BuildRHIDirect3DMesh *>(mesh);

	DX::RHIGetD3DDeviceContext()->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);

	mesh_internal->vertexbuffer->Bind();
	static_cast<BuildRHIDirect3D11Shader *>(shader)->Bind(RHI_INPUTSHADER_WORLD);

	
	//	for (int i = 0; i < RHIMAX_BOUNDTEXTURES; i++)
	//	{
	//		if (rhiPrivate.boundTextures[i] != NULL)
	//		{
	//			const BuildRHITextureDirect3D11 *rhiImage = static_cast<const BuildRHITextureDirect3D11 *>(rhiPrivate.boundTextures[i]);
	//			DX::RHIGetD3DDeviceContext()->PSSetShaderResources(i, 1, &rhiImage->resourceView);
	//		}
	//	}
	DX::RHIGetD3DDeviceContext()->Draw(numVertexes, startVertex);
	rhiPrivate.ResetContext();
}

void BuildRHI::DrawIndexedQuad(BuildRHIShader *shader, BuildRHIMesh *mesh, int startVertex, int startIndex, int numIndexes)
{
	BuildRHIDirect3DMesh *mesh_internal = static_cast<BuildRHIDirect3DMesh *>(mesh);

	mesh_internal->indexBuffer->Bind();
	mesh_internal->vertexbuffer->Bind();
	static_cast<BuildRHIDirect3D11Shader *>(shader)->Bind(RHI_INPUTSHADER_WORLD);


	//	for (int i = 0; i < RHIMAX_BOUNDTEXTURES; i++)
	//	{
	//		if (rhiPrivate.boundTextures[i] != NULL)
	//		{
	//			const BuildRHITextureDirect3D11 *rhiImage = static_cast<const BuildRHITextureDirect3D11 *>(rhiPrivate.boundTextures[i]);
	//			DX::RHIGetD3DDeviceContext()->PSSetShaderResources(i, 1, &rhiImage->resourceView);
	//		}
	//	}
	DX::RHIGetD3DDeviceContext()->DrawIndexed(numIndexes, startIndex, startVertex);
	rhiPrivate.ResetContext();
}