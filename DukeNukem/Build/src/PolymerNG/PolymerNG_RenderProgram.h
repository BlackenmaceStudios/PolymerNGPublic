// PolymerNG_RenderProgram.h
//

#pragma once

struct ShaderBinary
{
	void *vertex_shader;
	
};

//
// PolymerNGRenderProgram
//
class PolymerNGRenderProgram
{
public:
	// Loads in a render program.
	static PolymerNGRenderProgram *LoadRenderProgram(const char *fileName, bool useGUIVertexLayout);

	BuildRHIShader *GetRHIShader() { return rhiShader; }

private:
	PolymerNGRenderProgram(const char *name, BuildRHIShader *rhiShader);
	
private:
	void InitShader();

	const char *shaderName;
	BuildRHIShader *rhiShader;

};