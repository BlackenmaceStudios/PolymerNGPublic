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
	ID3D11Device1 *RHIGetD3DDevice();
	ID3D11DeviceContext1* RHIGetD3DDeviceContext();
	ID2D1Factory1 *RHIGetD2D1DeviceFactory3();
	void SetRHID3DDeviceContextOverride(ID3D11DeviceContext1 *g_override);
	ID2D1DeviceContext1 *RHIGet2D1DeviceContext2();
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
// BuildRHIDirect3D11GPUPerformanceCounter
//
class BuildRHIDirect3D11GPUPerformanceCounter : public BuildRHIGPUPerformanceCounter
{
public:
	BuildRHIDirect3D11GPUPerformanceCounter();

	virtual void			Begin();
	virtual void			End();
	virtual UINT64			GetTime();
private:
	ID3D11Query	*			rhiQueryStart;
	ID3D11Query	*			rhiQueryEnd;
	ID3D11Query	*			rhiQueryDisjoint;
};

class BuildRHIDirect3D11GPUOcclusionQuery : public BuildRHIGPUOcclusionQuery
{
public:
	BuildRHIDirect3D11GPUOcclusionQuery();
	virtual void			Begin();
	virtual void			End();
	virtual bool			IsVisible();
private:
	ID3D11Query	*			rhiQuery;
};

//
// BuildRHIDirect3D11Shader
//
class BuildRHIDirect3D11Shader : public BuildRHIShader
{
public:
	BuildRHIDirect3D11Shader();

	virtual bool LoadShader(BuildShaderTarget target, const char *buffer, int length, bool useGUIVertexLayout);
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

	virtual int			GetWidth() { return _width; }
	virtual int			GetHeight() { return _height; }
	virtual void		UploadRegion(int x, int y, int width, int height, const void *buffer)  const;

	ID3D11Resource      *GetTextureRHI();

	ID3D11Texture1D		*texture1D;
	ID3D11Texture2D		*texture2D;
	ID3D11ShaderResourceView *resourceView;

	BuildRHITextureFormat format;
	int _width;
	int _height;
	bool _isCubeMap;
};

//
// BuildRHIRenderTarget
//
class BuildRHIRenderTargetDirect3D11 : public BuildRHIRenderTarget
{
public:
	BuildRHIRenderTargetDirect3D11(BuildRHITexture *diffuseTexture, BuildRHITexture *depthTexture, BuildRHITexture *stencilTexture);

	void Bind(int slice = 0, bool shouldClear = true);

	virtual void AddRenderTarget(BuildRHITexture *image);
private:
	

	BuildRHITextureDirect3D11 *diffuseTexture[MAX_RENDER_TARGETS];
	BuildRHITextureDirect3D11 *depthTexture;
	BuildRHITextureDirect3D11 *stencilTexture;

	ID3D11RenderTargetView *renderTargetViewRHI[MAX_RENDER_TARGETS];
	ID3D11DepthStencilView *renderDepthStencilViewRHI[6];
	ID3D11ShaderResourceView *renderTargetRVRHI;
	

	int numRenderTargets;
};

void RHIAppSwitchBackToDeviceRenderBuffers();

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
// BuildRHICurrentRenderState
//
class BuildRHICurrentRenderState
{
public:
	BuildRHIDirect3D11Shader		*currentShader;
	const BuildRHITexture			*boundTextures[RHIMAX_BOUNDTEXTURES];
	ID3D11SamplerState				*samplerStates[RHIMAX_BOUNDTEXTURES];
	D3D11_PRIMITIVE_TOPOLOGY		current_topology;
	ID3D11PixelShader				*currentPixelShader;
	ID3D11GeometryShader			*currentGeomtryShader;
	ID3D11VertexShader				*currentVertexShader;
	ID3D11InputLayout				*currentLayout;
	BuildRHIConstantBuffer			*currentConstantBuffer[SHADER_BIND_NUMTYPES];
	BuildD3D11GPUBufferIndexBuffer  *currentIndexBuffer;
	BuildD3D11GPUBufferVertexBuffer *currentVertexBuffer;
};

//
// BuildRHIDirect3D11Private
//
class BuildRHIDirect3D11Private
{
public:
	BuildRHIDirect3DMesh			guiRHIMesh;
	BuildRHIDirect3DMesh			*currentMesh;

	BuildRHICurrentRenderState renderState;
	
	void					SetPixelShader(ID3D11PixelShader *pixelShader);
	void					SetVertexShader(ID3D11VertexShader *vertexShader);
	void					SetGeomtryShader(ID3D11GeometryShader *geometryShader);
	void					SetInputLayout(ID3D11InputLayout *layout);
	void					ResetContext();


public:
	ID3D11SamplerState		*pointSampleState;
	ID3D11SamplerState		*linearSampleState;
	ID3D11RasterizerState   *cullModeBuildState;
	ID3D11RasterizerState   *cullModeBuildFrontState;
	ID3D11RasterizerState   *cullModeBuildBackState;
	ID3D11BlendState1		*alphaBlendState;
	ID3D11BlendState1		*multiplyBlendState;
	ID3D11BlendState1		*additiveBlendState;

	ID3D11DeviceContext*	pDeferredContext_0;
	ID3D11DeviceContext1*	pDeferredContext;
};

void RHIProtected_SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY topology);
void RHIApiSetupContext(ID3D11DeviceContext1 *context);

extern BuildRHIDirect3D11Private rhiPrivate;