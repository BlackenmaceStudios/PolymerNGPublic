// Direct3D11_RHI_RenderTarget.cpp
//

#include "BuildRHI_Direct3D11.h"

//
// BuildRHI::AllocateRHIRenderTarget
//
BuildRHIRenderTarget *BuildRHI::AllocateRHIRenderTarget(BuildRHITexture *diffuseTexture, BuildRHITexture *depthTexture, BuildRHITexture *stencilTexture)
{
	return new BuildRHIRenderTargetDirect3D11(diffuseTexture, depthTexture, stencilTexture);
}

//
// BuildRHI::BindRenderTarget
//
void BuildRHI::BindRenderTarget(BuildRHIRenderTarget *renderTarget)
{
	BuildRHIRenderTargetDirect3D11 *rhiRenderTarget = static_cast<BuildRHIRenderTargetDirect3D11*>(renderTarget);

	if (rhiRenderTarget)
	{
		rhiRenderTarget->Bind();
	}
	else
	{
		RHIAppSwitchBackToDeviceRenderBuffers();
	}
}

//
// BuildRHIRenderTargetDirect3D11::BuildRHIRenderTargetDirect3D11
//
BuildRHIRenderTargetDirect3D11::BuildRHIRenderTargetDirect3D11(BuildRHITexture *diffuseTexture, BuildRHITexture *depthTexture, BuildRHITexture *stencilTexture)
{
	numRenderTargets = 0;

	this->depthTexture		= static_cast<BuildRHITextureDirect3D11*>(depthTexture);
	this->stencilTexture	= static_cast<BuildRHITextureDirect3D11*>(stencilTexture);

	AddRenderTarget(diffuseTexture);

	if (depthTexture != NULL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		HRESULT hr;

		ZeroMemory(&dsvDesc, sizeof(dsvDesc));
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		ID3D11Resource *depthRenderTargetRHI = this->depthTexture->GetTextureRHI();
		hr = DX::RHIGetD3DDevice()->CreateDepthStencilView(depthRenderTargetRHI, &dsvDesc, &renderDepthStencilViewRHI);
		if (FAILED(hr))
		{
			initprintf("BuildRHIRenderTargetDirect3D11: Failed to create render target(depth)!");
			return;
		}
	}
}

//
// BuildRHIRenderTargetDirect3D11::AddRenderTarget
//
void BuildRHIRenderTargetDirect3D11::AddRenderTarget(BuildRHITexture *diffuseTexture)
{
	if (diffuseTexture != NULL)
	{
		this->diffuseTexture[numRenderTargets] = static_cast<BuildRHITextureDirect3D11*>(diffuseTexture);

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		HRESULT result;
		ID3D11Resource *textureRenderTargetRHI = this->diffuseTexture[numRenderTargets]->GetTextureRHI();

		// Setup the description of the render target view.
		renderTargetViewDesc.Format = this->diffuseTexture[numRenderTargets]->format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		result = DX::RHIGetD3DDevice()->CreateRenderTargetView(textureRenderTargetRHI, &renderTargetViewDesc, &renderTargetViewRHI[numRenderTargets]);
		if (FAILED(result))
		{
			initprintf("BuildRHIRenderTargetDirect3D11: Failed to create render target!");
			return;
		}

		// Setup the description of the shader resource view.
		shaderResourceViewDesc.Format = this->diffuseTexture[numRenderTargets]->format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		result = DX::RHIGetD3DDevice()->CreateShaderResourceView(textureRenderTargetRHI, &shaderResourceViewDesc, &renderTargetRVRHI);
		if (FAILED(result))
		{
			initprintf("BuildRHIRenderTargetDirect3D11: Failed to create render target(RV)!");
			return;
		}

		numRenderTargets++;
	}
}

//
// BuildRHIRenderTargetDirect3D11::Bind
//
void BuildRHIRenderTargetDirect3D11::Bind()
{
	DX::RHIGetD3DDeviceContext()->OMSetRenderTargets(numRenderTargets, &renderTargetViewRHI[0], renderDepthStencilViewRHI);
	for (int i = 0; i < numRenderTargets; i++)
	{
		if(i == 0)
			DX::RHIGetD3DDeviceContext()->ClearRenderTargetView(renderTargetViewRHI[i], DirectX::Colors::Blue);
		else
			DX::RHIGetD3DDeviceContext()->ClearRenderTargetView(renderTargetViewRHI[i], DirectX::Colors::Black);
	}
	DX::RHIGetD3DDeviceContext()->ClearDepthStencilView(renderDepthStencilViewRHI, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	CD3D11_VIEWPORT m_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		diffuseTexture[0]->GetWidth(),
		diffuseTexture[0]->GetHeight()
		);

	DX::RHIGetD3DDeviceContext()->RSSetViewports(1, &m_screenViewport);
}