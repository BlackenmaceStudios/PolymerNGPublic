// Direct3D11_RHI_shader.cpp
//

#include "BuildRHI_Direct3D11.h"

BuildRHIShader *BuildRHI::AllocateShaderObject()
{
	BuildRHIDirect3D11Shader *shader = new BuildRHIDirect3D11Shader();
	return shader;
}

BuildRHIDirect3D11Shader::BuildRHIDirect3D11Shader()
{
	m_vertexShader = NULL;
	m_geometryShader = NULL;
	m_pixelShader = NULL;
}

void BuildRHIDirect3D11Shader::Bind(BuildRHIDirect3D11InputShaderType shaderType)
{
	if (m_vertexShader != NULL)
	{
		if (shaderType == RHI_INPUTSHADER_GUI)
		{
			rhiPrivate.SetInputLayout(guiInputLayout);
		}
		else if (shaderType == RHI_INPUTSHADER_WORLD)
		{
			rhiPrivate.SetInputLayout(worldInputLayout);
		}
		else
		{
			initprintf("BuildRHIDirect3D11Shader::Bind: Unknown shader type\n");
			return;
		}

		rhiPrivate.SetVertexShader(m_vertexShader);
	}

	if (m_pixelShader != NULL)
	{
		rhiPrivate.SetPixelShader(m_pixelShader);
	}

	if (m_geometryShader != NULL)
	{
		rhiPrivate.SetGeomtryShader(m_geometryShader);
	}
	else
	{
		rhiPrivate.SetGeomtryShader(NULL);
	}
}

bool BuildRHIDirect3D11Shader::LoadShader(BuildShaderTarget target, const char *buffer, int length, bool useGUIVertexLayout)
{
	HRESULT hr = S_OK;

	switch (target)
	{
		case SHADER_TARGET_VERTEX:
			hr = DX::RHIGetD3DDevice()->CreateVertexShader(buffer, length, nullptr, &m_vertexShader);
			if (useGUIVertexLayout)
			{
				if (!FAILED(hr))
				{
					hr = DX::RHIGetD3DDevice()->CreateInputLayout(guiModelInputElementDesc, 2, buffer, length, &guiInputLayout);
				}
			}
			else
			{
				if (!FAILED(hr))
				{
					hr = DX::RHIGetD3DDevice()->CreateInputLayout(worldModelInputElementDesc, 3, buffer, length, &worldInputLayout);
				}
			}
			break;

		case SHADER_TARGET_GEOMETRY:
			hr = DX::RHIGetD3DDevice()->CreateGeometryShader(buffer, length, nullptr, &m_geometryShader);
			break;

		case SHADER_TARGET_FRAGMENT:
			hr = DX::RHIGetD3DDevice()->CreatePixelShader(buffer, length, nullptr, &m_pixelShader);
			break;

		default:
			hr = S_FALSE;
			break;
	}



	return hr == S_OK;
}