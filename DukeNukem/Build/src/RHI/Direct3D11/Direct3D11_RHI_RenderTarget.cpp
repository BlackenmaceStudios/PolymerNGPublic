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
void BuildRHI::BindRenderTarget(BuildRHIRenderTarget *renderTarget, int slice, bool shouldClear)
{
	BuildRHIRenderTargetDirect3D11 *rhiRenderTarget = static_cast<BuildRHIRenderTargetDirect3D11*>(renderTarget);

	if (rhiRenderTarget)
	{
		rhiRenderTarget->Bind(slice, shouldClear);
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

		if (!this->depthTexture->_isCubeMap)
		{
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

			ID3D11Resource *depthRenderTargetRHI = this->depthTexture->GetTextureRHI();
			hr = DX::RHIGetD3DDevice()->CreateDepthStencilView(depthRenderTargetRHI, &dsvDesc, &renderDepthStencilViewRHI[0]);
			if (FAILED(hr))
			{
				initprintf("BuildRHIRenderTargetDirect3D11: Failed to create render target(depth)!");
				return;
			}
		}
		else
		{
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = 1;
			for (int i = 0; i < 6; i++)
			{
				dsvDesc.Texture2DArray.FirstArraySlice = i;

				ID3D11Resource *depthRenderTargetRHI = this->depthTexture->GetTextureRHI();
				hr = DX::RHIGetD3DDevice()->CreateDepthStencilView(depthRenderTargetRHI, &dsvDesc, &renderDepthStencilViewRHI[i]);
				if (FAILED(hr))
				{
					initprintf("BuildRHIRenderTargetDirect3D11: Failed to create render target(depth)!");
					return;
				}
			}
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
void BuildRHIRenderTargetDirect3D11::Bind(int slice, bool shouldClear)
{
	DX::RHIGetD3DDeviceContext()->OMSetRenderTargets(numRenderTargets, &renderTargetViewRHI[0], renderDepthStencilViewRHI[slice]);
	if (shouldClear)
	{
		for (int i = 0; i < numRenderTargets; i++)
		{
			if (i == 0)
				DX::RHIGetD3DDeviceContext()->ClearRenderTargetView(renderTargetViewRHI[i], DirectX::Colors::Black);
			else
				DX::RHIGetD3DDeviceContext()->ClearRenderTargetView(renderTargetViewRHI[i], DirectX::Colors::Black);
		}

		if (renderDepthStencilViewRHI[slice])
		{
			DX::RHIGetD3DDeviceContext()->ClearDepthStencilView(renderDepthStencilViewRHI[slice], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		else if (slice > 0)
		{
			initprintf("BuildRHIRenderTargetDirect3D11::Bind: Slice without depth detected, if this is normal add use case here.\n");
		}
	}

	int viewportWidth = 0;
	int viewportHeight = 0;

	if (diffuseTexture[0])
	{
		viewportWidth = diffuseTexture[0]->GetWidth();
		viewportHeight = diffuseTexture[0]->GetHeight();
	}
	else
	{
		viewportWidth = depthTexture->GetWidth();
		viewportHeight = depthTexture->GetHeight();
	}

	CD3D11_VIEWPORT m_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		viewportWidth,
		viewportHeight
		);

	DX::RHIGetD3DDeviceContext()->RSSetViewports(1, &m_screenViewport);
}