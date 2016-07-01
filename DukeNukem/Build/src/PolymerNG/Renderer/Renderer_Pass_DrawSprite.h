// Renderer_Pass_DrawSprite.h
//

//
// SpriteFacingType
//
enum SpriteFacingType
{
	SPRITE_FACING_VERTICAL = 0,
	SPRITE_FACING_HORIZONTAL,
	SPRITE_FACING_NUM
};

struct VS_DRAWSPRITE_BUFFER
{
	Math::XMFLOAT4X4  mWorldViewProj;
	Math::XMFLOAT4X4   modelMatrixInverse;
	float viewPosition[4];
};

struct PS_DRAWSPRITE_BUFFER
{
	float shadeOffsetVisibility[4];
	float fogDensistyScaleEnd[4];
	float fogColor[4];
};

class RendererDrawPassDrawSprite : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:
	void CalculateTransformAndNormalsForSprite(SpriteFacingType spriteType, float4x4 modelMatrix);

	BuildRHIMesh *spriteRHIMesh;
	BuildRHIConstantBuffer		*drawSpriteConstantBuffer;
	BuildRHIConstantBuffer		*drawSpritePixelConstantBuffer;

	VS_DRAWSPRITE_BUFFER			drawSpriteBuffer;
	PS_DRAWSPRITE_BUFFER			psDrawSpriteBuffer;

	Build3DVertex				spriteVertexes[8];
	Build3DVertex				spriteVertexesGPU[8];

	Build3DPlane				sprite3DPlanes[SPRITE_FACING_NUM];
};
