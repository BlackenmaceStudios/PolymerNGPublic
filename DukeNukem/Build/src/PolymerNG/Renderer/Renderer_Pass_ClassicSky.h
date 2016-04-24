// Renderer_Pass_ClassicSky.h
//

struct VS_DRAWCLASSICSKY_BUFFER
{
	Math::XMFLOAT4X4  mWorldViewProj;
	//Math::XMFLOAT4X4   modelMatrix;
};

class RendererDrawPassDrawClassicSky : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:
	BuildRHIMesh *classicSkyRHIMesh;
	BuildRHIConstantBuffer		*classicSkyConstantBuffer;
	float						artskydata[16];

	VS_DRAWCLASSICSKY_BUFFER			drawClassicSkyBuffer;
};
