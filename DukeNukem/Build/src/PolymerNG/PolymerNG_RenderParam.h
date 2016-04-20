// PolymerNG_RenderParam.h
//

#pragma once


//
// PolymerRenderParam
//
class PolymerRenderParam
{
public:
	PolymerRenderParam(const char *name, const Math::Vector4 &defaultValue);
	PolymerRenderParam(const char *name, const Math::Matrix4 &defaultValue);
	PolymerRenderParam(const char *name, BuildImage *defaultValue, BuildImageType imageTypeOverride = IMAGETYPE_NONE);

	const char *		GetName() { return name;  }
	const char *		GetRHIType();

	Math::Matrix4				GetMat4() { return defaultValueMatrix4; }
	void				SetMat4(Math::Matrix4 &mat) { defaultValueMatrix4 = mat; }

	enum PolymerRenderParamType
	{
		PARAM_TYPE_VECTOR4,
		PARAM_TYPE_MATRIX4,
		PARAM_TYPE_TEXTURE
	};

	PolymerRenderParamType	GetType() { return type; }
private:
	PolymerRenderParamType		type;

	Math::Vector4		defaultValueVec4;
	Math::Matrix4		defaultValueMatrix4;

	BuildImage *		image;

	BuildImageType imageTypeOverride;

	const char *name;
};