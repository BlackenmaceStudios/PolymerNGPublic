// Direct3D12_RHI_Images.cpp
//

#include "BuildRHI_Direct3D11.h"

BuildRHITextureFormat BuildRHI::GetRHITextureFormat(BuildImageFormat format)
{
	switch (format)
	{
	case IMAGE_FORMAT_R8G8:
		return DXGI_FORMAT_R8G8_UNORM;
	case IMAGE_FORMAT_DEPTH:
		return DXGI_FORMAT_R24G8_TYPELESS;
	case  IMAGE_FORMAT_R8:
		return DXGI_FORMAT_R8_UNORM;
	case IMAGE_FORMAT_R16:
		return DXGI_FORMAT_R16_SINT;
	case IMAGE_FORMAT_RGBA32:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case IMAGE_FORMAT_DXT5:
		return DXGI_FORMAT_BC3_UNORM;
	}

	return DXGI_FORMAT_UNKNOWN;
}

int BuildRHI::GetImageBitsFromTextureFormat(BuildRHITextureFormat format)
{
	switch (format)
	{
	case  DXGI_FORMAT_R8_UNORM:
		return 1;
	case DXGI_FORMAT_BC3_UNORM:
		return 4;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return 4;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return 4;
	case DXGI_FORMAT_R8G8_UNORM:
		return 2;
	case DXGI_FORMAT_R16_SINT:
		return 2;
	}

	return -1;
}

const BuildRHITexture* BuildRHI::LoadTextureFromMemory(const std::wstring &textureName, size_t Width, size_t Height, BuildRHITextureFormat Format, const void* InitData, bool allowCPUWrites, bool allowCPUReads, bool isRenderTargetImage)
{
	BuildRHITextureDirect3D11 *textureRHI = new BuildRHITextureDirect3D11();

	bool isDepthStencilFormat = Format == DXGI_FORMAT_D24_UNORM_S8_UINT || Format == DXGI_FORMAT_R24G8_TYPELESS;

	if (Width == 0 && Height == 0)
	{
		Width = 1;
		Height = 1;
	}

	if (Height == 0)
	{
		D3D11_TEXTURE1D_DESC desc;
		desc.Width = Width;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = Format;
		if (!allowCPUWrites)
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
		}
		else
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA subres;
		subres.pSysMem = InitData;
		subres.SysMemPitch = Width * GetImageBitsFromTextureFormat(Format);
		subres.SysMemSlicePitch = 0;

		HRESULT result = DX::RHIGetD3DDevice()->CreateTexture1D(&desc, &subres, &textureRHI->texture1D);
		if (FAILED(result))
		{
			initprintf("Failed to create Texture2D");			
		}

		result = DX::RHIGetD3DDevice()->CreateShaderResourceView(textureRHI->texture1D, NULL, &textureRHI->resourceView);
		if (FAILED(result))
		{
			initprintf("Failed to create resource view");
		}
	}
	else
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = Width;
		desc.Height = Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		if (allowCPUReads)
		{
			desc.Usage = D3D11_USAGE_STAGING;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		}
		else if (!allowCPUWrites)
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
		}
		else
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		if (allowCPUReads)
		{
			desc.BindFlags = 0;
		}
		else if (!isDepthStencilFormat)
		{
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		}
		else
		{
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		}

		if (isRenderTargetImage)
		{
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}

		desc.MiscFlags = 0;

		HRESULT result;
		if (InitData != NULL)
		{
			D3D11_SUBRESOURCE_DATA subres;
			subres.pSysMem = InitData;
			subres.SysMemPitch = Width * GetImageBitsFromTextureFormat(Format);
			subres.SysMemSlicePitch = 0;

			result = DX::RHIGetD3DDevice()->CreateTexture2D(&desc, &subres, &textureRHI->texture2D);
			if (FAILED(result))
			{
				initprintf("Failed to create Texture2D");
			}
		}
		else
		{
			HRESULT result = DX::RHIGetD3DDevice()->CreateTexture2D(&desc, nullptr, &textureRHI->texture2D);
			if (FAILED(result))
			{
				initprintf("Failed to create Texture2D");
			}
		}

		if (!isDepthStencilFormat && !allowCPUReads)
		{
			result = DX::RHIGetD3DDevice()->CreateShaderResourceView(textureRHI->texture2D, NULL, &textureRHI->resourceView);
			if (FAILED(result))
			{
				initprintf("Failed to create resource view");
			}
		}
		else if(isDepthStencilFormat)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
			sr_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			sr_desc.Texture2D.MostDetailedMip = 0;
			sr_desc.Texture2D.MipLevels = -1;
			result = DX::RHIGetD3DDevice()->CreateShaderResourceView(textureRHI->texture2D, &sr_desc, &textureRHI->resourceView);
			if (FAILED(result))
			{
				initprintf("Failed to create resource view");
			}
		}
	}

	textureRHI->format = Format;

	textureRHI->_width = Width;
	textureRHI->_height = Height;

	return textureRHI;
}

void BuildRHI::ReadBackPixelsFromImage(const BuildRHITexture *image, byte *buffer)
{
	BuildRHITextureDirect3D11 *rhiImage = static_cast<BuildRHITextureDirect3D11 *>((BuildRHITexture *)image);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	DX::RHIGetD3DDeviceContext()->Map(rhiImage->GetTextureRHI(), 0, D3D11_MAP_READ, 0, &mappedResource);

	memcpy(buffer, mappedResource.pData, rhiImage->GetWidth() * rhiImage->GetHeight() * GetImageBitsFromTextureFormat(rhiImage->format));

	DX::RHIGetD3DDeviceContext()->Unmap(rhiImage->GetTextureRHI(), 0);
}

void BuildRHI::CopyImageToAnotherImage(const BuildRHITexture *src, const BuildRHITexture *dst, int x, int y, int width, int height)
{
	BuildRHITextureDirect3D11 *rhiSrcImage = static_cast<BuildRHITextureDirect3D11 *>((BuildRHITexture *)src);
	BuildRHITextureDirect3D11 *rhiDstImage = static_cast<BuildRHITextureDirect3D11 *>((BuildRHITexture *)dst);

	D3D11_BOX srcBox;
	srcBox.left = x;
	srcBox.right = width;
	srcBox.top = y;
	srcBox.bottom = height;
	srcBox.front = 0;
	srcBox.back = 1;

	DX::RHIGetD3DDeviceContext()->CopySubresourceRegion(rhiDstImage->GetTextureRHI(), 0, 0, 0, 0, rhiSrcImage->GetTextureRHI(), 0, &srcBox);
}

BuildRHITextureDirect3D11::BuildRHITextureDirect3D11()
{
	texture1D = NULL;
	texture2D = NULL;
}

void BuildRHITextureDirect3D11::UploadRegion(int x, int y, int width, int height, const void *buffer) const
{
	ID3D11Resource *resourceToUpdate = NULL;

	if (texture1D)
		resourceToUpdate = texture1D;
	else if (texture2D)
		resourceToUpdate = texture2D;

	if (resourceToUpdate == NULL)
	{
		initprintf("UploadRegion: resourceToUpdate == NULL\n");
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	DX::RHIGetD3DDeviceContext()->Map(resourceToUpdate, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
	byte *buffer2 = (byte *)buffer;
	int rowSpan = width * rhi.GetImageBitsFromTextureFormat(format);
	for (UINT i = 0; i < height; ++i)
	{
		memcpy(mappedData, buffer2, rowSpan);
		mappedData += mappedResource.RowPitch;
		buffer2 += rowSpan;
	}
//	memcpy(mappedResource.pData, buffer, (width * height * rhi.GetImageBitsFromTextureFormat(format)));

	DX::RHIGetD3DDeviceContext()->Unmap(resourceToUpdate, 0);
}

ID3D11Resource *BuildRHITextureDirect3D11::GetTextureRHI()
{
	if (texture1D)
		return texture1D;

	if (texture2D)
		return texture2D;

	return NULL;
}