#pragma once

struct VS_DRAWWORLD_BUFFER
{
	Math::XMFLOAT4X4  mWorldViewProj;
};

class RendererDrawPassDrawWorld : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw( const BuildRenderCommand &command);
private:
	void						DrawPlane(const BaseModel *model, const Build3DPlane *plane);

	BuildRHIConstantBuffer		*drawWorldConstantBuffer;
	VS_DRAWWORLD_BUFFER			drawWorldBuffer;
};