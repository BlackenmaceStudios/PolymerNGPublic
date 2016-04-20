// Direct3D12_RHI_Shader.cpp
//

#include "../BuildRHI.h"

#include <d3d11shader.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>

typedef HRESULT 
(*D3DCompileFunc_t)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
	_In_ SIZE_T SrcDataSize,
	_In_opt_ LPCSTR pSourceName,
	_In_reads_opt_(_Inexpressible_(pDefines->Name != NULL)) CONST D3D_SHADER_MACRO* pDefines,
	_In_opt_ ID3DInclude* pInclude,
	_In_opt_ LPCSTR pEntrypoint,
	_In_ LPCSTR pTarget,
	_In_ UINT Flags1,
	_In_ UINT Flags2,
	_Out_ ID3DBlob** ppCode,
	_Out_opt_ ID3DBlob** ppErrorMsgs);

D3DCompileFunc_t rhiShaderCompile;

void LoadDirect3DCompileDLL()
{
	static HMODULE direct3dModule = NULL;

	if (direct3dModule != NULL && rhiShaderCompile != NULL)
		return;

	direct3dModule = LoadLibrary(D3DCOMPILER_DLL_A);
	if (direct3dModule == NULL)
	{
		assert(false);
	}

	rhiShaderCompile = (D3DCompileFunc_t)GetProcAddress(direct3dModule, "D3DCompile");
	if (!rhiShaderCompile)
	{
		assert(false);
	}
}

bool BuildRHI::CompileShader(const char *shader_name, BuildShaderTarget target, const char *buffer, int length, BuildRHIBlob &blob)
{
	LoadDirect3DCompileDLL();

	HRESULT hr;
	const char *target_name = "";
	ID3DBlob *shader_blob, *error_blob;
	
	switch (target)
	{
		case SHADER_TARGET_VERTEX:
			target_name = "vs_5_1";
			break;

		case SHADER_TARGET_FRAGMENT:
			target_name = "ps_5_1";
			break;
	}

	initprintf("Compiling %s\n", shader_name);
	hr = rhiShaderCompile(buffer, length, shader_name, NULL, NULL, "main", target_name, 0, 0, &shader_blob, &error_blob);
	if(hr != S_OK)
	{
		std::string compile_messages = (const char *)error_blob->GetBufferPointer();
		error_blob->Release();
		initprintf("SHADER COMPILE ERROR\n");
		initprintf(compile_messages.c_str());
		return false;
	}

	if (error_blob != NULL)
	{
		std::string compile_messages = (const char *)error_blob->GetBufferPointer();
		if (compile_messages.size() > 0)
		{
			initprintf("While compiling shader\n");
			initprintf(compile_messages.c_str());
		}
		error_blob->Release();
	}

	blob.SetMemory(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize());
	shader_blob->Release();

	return true;
}