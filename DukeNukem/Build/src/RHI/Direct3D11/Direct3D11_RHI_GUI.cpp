// Direct3D11_RHI_GUI.cpp
//

#include "BuildRHI_Direct3D11.h"

void BuildRHI::DrawUnoptimized2DQuad( BuildRHIUIVertex *vertexes)
{
	RHIProtected_SetPrimitiveType(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//	DX::RHIGetD3DDeviceContext()->IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);

	// This is allowed.
	if (vertexes != NULL)
	{
		rhiPrivate.guiRHIMesh.vertexbuffer->UpdateBuffer(vertexes, 4 * sizeof(BuildRHIUIVertex), 0);
		rhiPrivate.guiRHIMesh.vertexbuffer->Bind();
	}

	rhiPrivate.renderState.currentShader->Bind(RHI_INPUTSHADER_GUI);
//	for (int i = 0; i < RHIMAX_BOUNDTEXTURES; i++)
//	{
//		if (rhiPrivate.boundTextures[i] != NULL)
//		{
//			const BuildRHITextureDirect3D11 *rhiImage = static_cast<const BuildRHITextureDirect3D11 *>(rhiPrivate.boundTextures[i]);
//			DX::RHIGetD3DDeviceContext()->PSSetShaderResources(i, 1, &rhiImage->resourceView);
//		}
//	}
	DX::RHIGetD3DDeviceContext()->Draw(4, 0);
	//rhiPrivate.ResetContext();
}