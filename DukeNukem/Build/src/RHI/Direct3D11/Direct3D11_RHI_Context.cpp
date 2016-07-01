// Direct3D11_RHI_Context.cpp
//

#include "BuildRHI_Direct3D11.h"

void RHIProtected_SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	if (rhiPrivate.renderState.current_topology == topology)
		return;

	rhiPrivate.renderState.current_topology = topology;
	DX::RHIGetD3DDeviceContext()->IASetPrimitiveTopology(rhiPrivate.renderState.current_topology);
}

void BuildRHIDirect3D11Private::ResetContext()
{
	for (int i = 0; i < RHIMAX_BOUNDTEXTURES; i++)
	{
		rhiPrivate.renderState.boundTextures[i] = NULL;
		rhiPrivate.renderState.samplerStates[i] = NULL;
	}
	
	rhiPrivate.renderState.current_topology = (D3D11_PRIMITIVE_TOPOLOGY)-1;
	rhiPrivate.renderState.currentPixelShader = NULL;
	rhiPrivate.renderState.currentGeomtryShader = NULL;
	rhiPrivate.renderState.currentVertexShader = NULL;
	rhiPrivate.renderState.currentLayout = NULL;
	for (int i = 0; i < SHADER_BIND_NUMTYPES; i++)
	{
		rhiPrivate.renderState.currentConstantBuffer[i] = NULL;
	}
	rhiPrivate.renderState.currentVertexBuffer = NULL;
	rhiPrivate.renderState.currentIndexBuffer = NULL;
}

void BuildRHI::SetBlendState(BuildBlendState blendstate)
{
	switch (blendstate)
	{
		case BLENDSTATE_ALPHA:
			DX::RHIGetD3DDeviceContext()->OMSetBlendState(rhiPrivate.alphaBlendState, 0, 0xffffffff);
			break;

		case BLENDSTATE_ADDITIVE:
			DX::RHIGetD3DDeviceContext()->OMSetBlendState(rhiPrivate.additiveBlendState, 0, 0xffffffff);
			break;

		default:
			initprintf("BuildRHI::SetBlendState: Unknown blend state!!!");
			break;
	}
}

void BuildRHI::SetFaceCulling(BuildRHIFaceCulling cullMode)
{
	switch (cullMode)
	{
		case CULL_FACE_NONE:
			DX::RHIGetD3DDeviceContext()->RSSetState(rhiPrivate.cullModeBuildState);
			break;
		case CULL_FACE_BACK:
			DX::RHIGetD3DDeviceContext()->RSSetState(rhiPrivate.cullModeBuildBackState);
			break;
		case CULL_FACE_FRONT:
			DX::RHIGetD3DDeviceContext()->RSSetState(rhiPrivate.cullModeBuildFrontState);
			break;
		default:
			initprintf("BuildRHI::SetFaceCulling: Unknown cull mode!");
			break;
	}
}

void BuildRHI::ToggleDeferredRenderContext(bool enable)
{
//	return;
//
//	if (enable)
//	{
//		DX::SetRHID3DDeviceContextOverride(rhiPrivate.pDeferredContext);
//		RHIApiSetupContext(rhiPrivate.pDeferredContext);
//		rhiPrivate.ResetContext();
//	}
//	else
//	{
//		// Reset the render context.
//		DX::SetRHID3DDeviceContextOverride(NULL);
//
//		ID3D11CommandList* pd3dCommandList = NULL;
//		HRESULT hr;
//		hr = rhiPrivate.pDeferredContext->FinishCommandList(FALSE, &pd3dCommandList);
//
//		DX::RHIGetD3DDeviceContext()->ExecuteCommandList(pd3dCommandList, FALSE);
//		rhiPrivate.ResetContext();
//	}
}

void BuildRHI::SetImageForContext(int rootIndex, const BuildRHITexture *image, bool useLinearFilter)
{
	const BuildRHITextureDirect3D11 *rhiImage = static_cast<const BuildRHITextureDirect3D11 *>(image);
	ID3D11SamplerState *rhiSamplerState = rhiPrivate.pointSampleState;
	//if (useLinearFilter)
	//{
	//	rhiSamplerState = rhiPrivate.linearSampleState;
	//}
	//if (rhiPrivate.renderState.boundTextures[rootIndex] == image)
	//{
	//	if (rhiPrivate.renderState.samplerStates[rootIndex] == rhiSamplerState)
	//	{
	//		return;
	//	}
	//}

	DX::RHIGetD3DDeviceContext()->PSSetShaderResources(rootIndex, 1, &rhiImage->resourceView);
	DX::RHIGetD3DDeviceContext()->PSSetSamplers(rootIndex, 1, &rhiSamplerState);
	rhiPrivate.renderState.boundTextures[rootIndex] = image;
	rhiPrivate.renderState.samplerStates[rootIndex] = rhiSamplerState;
}

void BuildRHI::SetShader(BuildRHIShader *shader)
{
	rhiPrivate.renderState.currentShader = static_cast<BuildRHIDirect3D11Shader *>(shader);
}

void BuildRHIDirect3D11Private::SetPixelShader(ID3D11PixelShader *pixelShader)
{
	if (pixelShader != rhiPrivate.renderState.currentPixelShader)
	{
		DX::RHIGetD3DDeviceContext()->PSSetShader(pixelShader, NULL, 0);
		rhiPrivate.renderState.currentPixelShader = pixelShader;
	}
}

void BuildRHIDirect3D11Private::SetGeomtryShader(ID3D11GeometryShader *geometryShader)
{
	if (geometryShader != rhiPrivate.renderState.currentGeomtryShader)
	{
		DX::RHIGetD3DDeviceContext()->GSSetShader(geometryShader, NULL, 0);
		rhiPrivate.renderState.currentGeomtryShader = geometryShader;
	}
}

void BuildRHIDirect3D11Private::SetVertexShader(ID3D11VertexShader *vertexShader)
{
	if (vertexShader != rhiPrivate.renderState.currentVertexShader)
	{
		DX::RHIGetD3DDeviceContext()->VSSetShader(vertexShader, NULL, 0);
		rhiPrivate.renderState.currentVertexShader = vertexShader;
	}
}

void BuildRHIDirect3D11Private::SetInputLayout(ID3D11InputLayout *layout)
{
	if (layout != rhiPrivate.renderState.currentLayout)
	{
		DX::RHIGetD3DDeviceContext()->IASetInputLayout(layout);
		rhiPrivate.renderState.currentLayout = layout;
	}
}

void BuildRHI::SetConstantBuffer(int index, BuildRHIConstantBuffer *constantBuffer, BuildShaderBindTarget target)
{
	if (rhiPrivate.renderState.currentConstantBuffer[target] != constantBuffer)
	{
		static_cast<BuildD3D11GPUBufferConstantBuffer *>(constantBuffer)->Bind(target);
		rhiPrivate.renderState.currentConstantBuffer[target] = constantBuffer;
	}
}

void BuildRHI::DrawUnoptimizedQuad(BuildRHIShader *shader, BuildRHIMesh *mesh, int startVertex, int numVertexes)
{
	BuildRHIDirect3DMesh *mesh_internal = static_cast<BuildRHIDirect3DMesh *>(mesh);

	//DX::RHIGetD3DDeviceContext()->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);

	mesh_internal->vertexbuffer->Bind();
	static_cast<BuildRHIDirect3D11Shader *>(shader)->Bind(RHI_INPUTSHADER_WORLD);

	RHIProtected_SetPrimitiveType(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	
	//	for (int i = 0; i < RHIMAX_BOUNDTEXTURES; i++)
	//	{
	//		if (rhiPrivate.boundTextures[i] != NULL)
	//		{
	//			const BuildRHITextureDirect3D11 *rhiImage = static_cast<const BuildRHITextureDirect3D11 *>(rhiPrivate.boundTextures[i]);
	//			DX::RHIGetD3DDeviceContext()->PSSetShaderResources(i, 1, &rhiImage->resourceView);
	//		}
	//	}
	DX::RHIGetD3DDeviceContext()->Draw(numVertexes, startVertex);
	//rhiPrivate.ResetContext();
}

void BuildRHI::DrawIndexedQuad(BuildRHIShader *shader, BuildRHIMesh *mesh, int startVertex, int startIndex, int numIndexes)
{
	BuildRHIDirect3DMesh *mesh_internal = static_cast<BuildRHIDirect3DMesh *>(mesh);

	mesh_internal->indexBuffer->Bind();
	mesh_internal->vertexbuffer->Bind();
	static_cast<BuildRHIDirect3D11Shader *>(shader)->Bind(RHI_INPUTSHADER_WORLD);
	RHIProtected_SetPrimitiveType(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//	for (int i = 0; i < RHIMAX_BOUNDTEXTURES; i++)
	//	{
	//		if (rhiPrivate.boundTextures[i] != NULL)
	//		{
	//			const BuildRHITextureDirect3D11 *rhiImage = static_cast<const BuildRHITextureDirect3D11 *>(rhiPrivate.boundTextures[i]);
	//			DX::RHIGetD3DDeviceContext()->PSSetShaderResources(i, 1, &rhiImage->resourceView);
	//		}
	//	}
	DX::RHIGetD3DDeviceContext()->DrawIndexed(numIndexes, startIndex, startVertex);
	//rhiPrivate.ResetContext();
}