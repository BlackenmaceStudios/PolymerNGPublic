#pragma once

struct VS_DRAWWORLD_BUFFER
{
	Math::XMFLOAT4X4  mWorldViewProj;
	Math::XMFLOAT4X4  mWorldView;
};

struct PS_CONSTANT_BUFFER
{
	float shadeOffsetVisibility[4];
	float fogDensistyScaleEnd[4];
	float fogColor[4];
};

class RendererDrawPassDrawWorld : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw( const BuildRenderCommand &command);
private:
	void						DrawPlane(BuildRHIMesh *rhiMesh, const BaseModel *model, const Build3DPlane *plane);

	BuildRHIConstantBuffer		*drawWorldPixelConstantBuffer;
	BuildRHIConstantBuffer		*drawWorldConstantBuffer;
	VS_DRAWWORLD_BUFFER			drawWorldBuffer;
	PS_CONSTANT_BUFFER			drawWorldPixelBuffer;
};