#pragma once

struct VS_DRAWWORLD_BUFFER
{
	Math::XMFLOAT4X4  mWorldViewProj;
	Math::XMFLOAT4X4  mWorldView;
	Math::XMFLOAT4X4 inverseViewMatrix;
	float viewPosition[4];
	Math::XMFLOAT4X4  mProjectionMatrix;
};

struct PS_CONSTANT_BUFFER
{
	float shadeOffsetVisibility[4];
	float fogDensistyScaleEnd[4];
	float fogColor[4];
	float normal[4];
	float tangent[4];
};

class RendererDrawPassDrawWorld : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw( const BuildRenderCommand &command);

	// Allows the main render class to start binding the buffers.
	void						BindDrawWorldRenderTarget(bool enable);

	BuildImage					*GetWorldDepthImage() {
		return depthRenderBuffer_copied;
	}

	// Returns the render target used in drawing the world.
	PolymerNGRenderTarget		*GetDrawWorldRenderTarget() { return renderTarget; }
private:
	void						DrawPlane(BuildRHIMesh *rhiMesh, const BaseModel *model, const Build3DPlane *plane);

	BuildRHIConstantBuffer		*drawWorldPixelConstantBuffer[2];
	BuildRHIConstantBuffer		*drawWorldConstantBuffer;
	VS_DRAWWORLD_BUFFER			drawWorldBuffer;
	PS_CONSTANT_BUFFER			drawWorldPixelBuffer;

	BuildImage					*depthRenderBuffer_copied;

	PolymerNGRenderTarget		*renderTarget;
};