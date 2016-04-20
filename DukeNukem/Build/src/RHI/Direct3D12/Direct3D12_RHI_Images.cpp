// Direct3D12_RHI_Images.cpp
//

#include "../BuildRHI.h"


BuildRHITextureFormat BuildRHI::GetRHITextureFormat(BuildImageFormat format)
{
	switch (format)
	{
		case  IMAGE_FORMAT_R8:
			return DXGI_FORMAT_R8_UNORM;
		case IMAGE_FORMAT_RGBA32:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	return DXGI_FORMAT_UNKNOWN;
}

const BuildRHITexture* BuildRHI::LoadTextureFromMemory(const std::wstring &textureName, size_t Width, size_t Height, BuildRHITextureFormat Format, const void* InitData, bool allowCPUWrites)
{
	return TextureManager::LoadTextureFromMemory(textureName, Width, Height, Format, (const void *)InitData, allowCPUWrites);
}

const BuildRHITexture* BuildRHI::LoadTexture3DFromMemory(const std::wstring &textureName, size_t Width, size_t Height, size_t Depth, BuildRHITextureFormat Format, const void* InitData, bool allowCPUWrites)
{
	return TextureManager::LoadTexture3DFromMemory(textureName, Width, Height, Depth, Format, (const void *)InitData, allowCPUWrites);
}