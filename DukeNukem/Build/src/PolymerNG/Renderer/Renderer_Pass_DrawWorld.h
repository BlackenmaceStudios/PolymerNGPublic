#pragma once

bool IsTransparentTile(int idx);

struct VS_DRAWWORLD_BUFFER
{
	float4x4  mWorldViewProj;
	float4x4  mWorldView;
	float4x4 inverseViewMatrix;
	float viewPosition[4];
	float4x4  mProjectionMatrix;
};

struct PS_CONSTANT_BUFFER
{
	float shadeOffsetVisibility[4];
	float fogDensistyScaleEnd[4];
	float fogColor[4];
	float normal[4];
	float tangent[4];
	float ambient[4];
};

class RendererDrawPassDrawWorld : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw( const BuildRenderCommand &command);
	virtual void				DrawTrans(const BuildRenderCommand &command);

	// Allows the main render class to start binding the buffers.
	void						BindDrawWorldRenderTarget(bool enable, bool shouldClear);

	BuildImage					*GetWorldDepthImage() {
		return depthRenderBuffer_copied;
	}

	// Returns the render target used in drawing the world.
	PolymerNGRenderTarget		*GetDrawWorldRenderTarget() { return renderTarget; }

	BuildImage					*GetPreviousRenderFrame() { return diffuseRenderBufferPrev; }
private:
	void						DrawPlane(BuildRHIMesh *rhiMesh, const BaseModel *model, const Build3DPlane *plane, bool isTransparent);

	BuildRHIConstantBuffer		*drawWorldPixelConstantBuffer[2];
	BuildRHIConstantBuffer		*drawWorldConstantBuffer;
	VS_DRAWWORLD_BUFFER			drawWorldBuffer;
	PS_CONSTANT_BUFFER			drawWorldPixelBuffer;

	BuildImage					*depthRenderBuffer_copied;

	BuildImage					*diffuseRenderBufferPrev;

	PolymerNGRenderTarget		*renderTarget;
private:
	std::vector<const Build3DPlane *> transparentPlaneList;
	std::vector<const Build3DPlane *> glowPlaneList;
};