// Renderer_Pass_DrawSprite.h
//

struct VS_DRAWSPRITE_BUFFER
{
	Math::XMFLOAT4X4  mWorldViewProj;
	//Math::XMFLOAT4X4   modelMatrix;
};

class RendererDrawPassDrawSprite : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:
	BuildRHIMesh *spriteRHIMesh;
	BuildRHIConstantBuffer		*drawSpriteConstantBuffer;

	VS_DRAWSPRITE_BUFFER			drawSpriteBuffer;
};
