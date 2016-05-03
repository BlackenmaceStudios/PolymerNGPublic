// Renderer_DrawUI.h
//

struct PS_DRAWUI_BUFFER
{
	float modulationColor[4];

};

class RendererDrawPassDrawUI : public RendererDrawPassBase
{
public:
	// Initializes the render draw pass.
	virtual void				Init();

	// Draws the Build Render Command
	virtual void				Draw(const BuildRenderCommand &command);
private:
	PS_DRAWUI_BUFFER			drawUIBuffer;
	BuildRHIConstantBuffer		*drawUIConstantBuffer;
};
