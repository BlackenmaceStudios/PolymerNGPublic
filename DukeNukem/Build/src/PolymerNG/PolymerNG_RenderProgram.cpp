// PolymerNG_RenderProgram.cpp
//

#include "PolymerNG_local.h"



/*
=====================
PolymerNGRenderProgram::LoadRenderProgram
=====================
*/
PolymerNGRenderProgram *PolymerNGRenderProgram::LoadRenderProgram(const char *fileName, bool useGUIVertexLayout)
{
	std::string fullPathFileName = "../";
	fullPathFileName += fileName;

	// Load in the vertex shader.
	BuildRHIShader *shader = rhi.AllocateShaderObject();

	{
		std::string tempFullFileName = fullPathFileName;
		tempFullFileName += "_vertex.cso";

		BuildFile *file = BuildFile::OpenFile(tempFullFileName.c_str(), BuildFile::BuildFile_Read);
		if (!file)
			return NULL;

		ScopedHeapMemory memory(file->Length());
		file->Read(memory.GetBuffer(), file->Length());
		shader->LoadShader(SHADER_TARGET_VERTEX, memory.GetBuffer(), file->Length(), useGUIVertexLayout);
		delete file;
	}

	// Load in the pixel shader.
	{
		std::string tempFullFileName = fullPathFileName;
		tempFullFileName += "_pixel.cso";

		BuildFile *file = BuildFile::OpenFile(tempFullFileName.c_str(), BuildFile::BuildFile_Read);
		if (!file)
			return NULL;

		ScopedHeapMemory memory(file->Length());
		file->Read(memory.GetBuffer(), file->Length());
		shader->LoadShader(SHADER_TARGET_FRAGMENT, memory.GetBuffer(), file->Length(), useGUIVertexLayout);
		delete file;
	}

	// Load in the geometry shader(if there is one).
	{
		std::string tempFullFileName = fullPathFileName;
		tempFullFileName += "_geometry.cso";

		BuildFile *file = BuildFile::OpenFile(tempFullFileName.c_str(), BuildFile::BuildFile_Read);
		if (file)
		{
			ScopedHeapMemory memory(file->Length());
			file->Read(memory.GetBuffer(), file->Length());
			shader->LoadShader(SHADER_TARGET_GEOMETRY, memory.GetBuffer(), file->Length(), useGUIVertexLayout);
			delete file;
		}
	}

	return new PolymerNGRenderProgram(fileName, shader);
}

/*
=====================
PolymerNGRenderProgram::PolymerNGRenderProgram
=====================
*/
PolymerNGRenderProgram::PolymerNGRenderProgram(const char *name, BuildRHIShader *rhiShader)
{
	this->shaderName = name;
	this->rhiShader = rhiShader;
	InitShader();
}

/*
=====================
PolymerNGRenderProgram::InitShader
=====================
*/
void PolymerNGRenderProgram::InitShader()
{

}