// BuildRHI_Direct3D11.h
//

#pragma once

#include <string>
#include "../BuildRHI.h"
#include "BuildRHI_Direct3D11_GPUBuffer.h"
#include <exception>

//
// Interfaces from the app.
//
namespace DX
{
	ID3D11Device3 *RHIGetD3DDevice();
	ID3D11DeviceContext3* RHIGetD3DDeviceContext();
	ID2D1Factory3 *RHIGetD2D1DeviceFactory3();
	ID2D1DeviceContext2 *RHIGet2D1DeviceContext2();
}

//
// BuildRHIDirect3D11InputShaderType
//
enum BuildRHIDirect3D11InputShaderType
{
	RHI_INPUTSHADER_GUI = 0,
	RHI_INPUTSHADER_WORLD
};

//
// BuildRHIDirect3D11Shader
//
class BuildRHIDirect3D11Shader : public BuildRHIShader
{
public:
	BuildRHIDirect3D11Shader();

	virtual bool LoadShader(BuildShaderTarget target, const char *buffer, int length);
	void Bind(BuildRHIDirect3D11InputShaderType shaderType);
private:
	ID3D11VertexShader		*m_vertexShader;
	ID3D11PixelShader		*m_pixelShader;
	ID3D11GeometryShader	*m_geometryShader;

	ID3D11InputLayout *guiInputLayout;
	ID3D11InputLayout *worldInputLayout;
};

//
// BuildRHITexture
//
class BuildRHITextureDirect3D11 : public BuildRHITexture
{
public:
	BuildRHITextureDirect3D11();

	virtual int			GetWidth() { return 0; }
	virtual int			GetHeight() { return 0; }
	virtual void		UploadRegion(int x, int y, int width, int height, const void *buffer)  const;

	ID3D11Texture1D		*texture1D;
	ID3D11Texture2D		*texture2D;
	ID3D11ShaderResourceView *resourceView;

	BuildRHITextureFormat format;
};

//
// BuildRHIDirect3DMesh
//
class BuildRHIDirect3DMesh : public BuildRHIMesh
{
public:
	BuildD3D11GPUBufferVertexBuffer *vertexbuffer;
	BuildD3D11GPUBufferIndexBuffer *indexBuffer;
};

#define RHIMAX_BOUNDTEXTURES		30

extern D3D11_INPUT_ELEMENT_DESC guiModelInputElementDesc[];
extern D3D11_INPUT_ELEMENT_DESC worldModelInputElementDesc[];

//
// BuildRHIDirect3D11Private
//
class BuildRHIDirect3D11Private
{
public:
	BuildRHIDirect3DMesh			guiRHIMesh;
	BuildRHIDirect3DMesh			*currentMesh;

	BuildRHIDirect3D11Shader *currentShader;
	const BuildRHITexture	*boundTextures[RHIMAX_BOUNDTEXTURES];

	void					SetPixelShader(ID3D11PixelShader *pixelShader);
	void					SetVertexShader(ID3D11VertexShader *vertexShader);
	void					SetGeomtryShader(ID3D11GeometryShader *geometryShader);
	void					SetInputLayout(ID3D11InputLayout *layout);
	void					ResetContext();


public:
	ID3D11SamplerState		*pointSampleState;
	ID3D11RasterizerState   *cullModeBuildState;
};

extern BuildRHIDirect3D11Private rhiPrivate;