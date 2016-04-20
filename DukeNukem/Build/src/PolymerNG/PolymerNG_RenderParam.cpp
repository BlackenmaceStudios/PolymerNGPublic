// PolymerNG_RenderParam.cpp
//

#include "PolymerNG_local.h"

PolymerRenderParam::PolymerRenderParam(const char *name, const Math::Vector4 &defaultValue)
{
	this->name = name;
	this->defaultValueVec4 = defaultValue;
	this->type = PARAM_TYPE_VECTOR4;
}

PolymerRenderParam::PolymerRenderParam(const char *name, const Math::Matrix4 &defaultValue)
{
	this->name = name;
	this->defaultValueMatrix4 = defaultValue;
	this->type = PARAM_TYPE_MATRIX4;
}

PolymerRenderParam::PolymerRenderParam(const char *name, BuildImage *defaultValue, BuildImageType imageTypeOverride)
{
	this->name = name;
	this->image = defaultValue;
	this->imageTypeOverride = imageTypeOverride;
	this->type = PARAM_TYPE_TEXTURE;
}

const char *PolymerRenderParam::GetRHIType()
{
	switch (type)
	{
		case PARAM_TYPE_VECTOR4:
			return "float4";

		case PARAM_TYPE_MATRIX4:
			return "float4x4";

		case PARAM_TYPE_TEXTURE:
			BuildImageType imageType = imageTypeOverride;
			
			if (image != NULL)
			{
				imageType = image->GetOpts().imageType;
			}

			if (imageType == IMAGETYPE_1D)
			{
				return "Texture1D";
			}
			else if (imageType == IMAGETYPE_2D)
			{
				return "Texture2D";
			}
			else if (imageType == IMAGETYPE_3D)
			{
				return "Texture3D";
			}
			return "Unknown Texture Type";
	}

	return NULL;
}