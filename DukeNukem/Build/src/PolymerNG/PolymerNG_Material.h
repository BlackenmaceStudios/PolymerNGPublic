// PolymerNG_Material.h
//

#pragma once

//
// PolymerNGMaterial
//
class PolymerNGMaterial
{
friend class PolymerNGMaterialManager;
public:
	BuildImage *GetDiffuseTexture()
	{
		return diffuseImage;
	}

	BuildImage *GetNormalMap()
	{
		return normalMap;
	}

	BuildImage *GetSpecularMap()
	{
		return specularImage;
	}

	size_t GetMaterialNameHash()
	{
		return name_hash;
	}
private:
	PolymerNGMaterial();

	void	SetNameAndHash(const char *name, size_t name_hash) { this->name = name; this->name_hash = name_hash; }

	void	SetDiffuseTexture(BuildImage *image)
	{
		diffuseImage = image;
	}

	void	SetNormalMap(BuildImage *image)
	{
		normalMap = image;
	}

	void	SetSpecularMap(BuildImage *image)
	{
		specularImage = image;
	}

private:
	std::string			name;
	size_t				name_hash;

	BuildImage			*diffuseImage;
	BuildImage			*specularImage;
	BuildImage			*normalMap;
	
};

__forceinline PolymerNGMaterial::PolymerNGMaterial()
{
	diffuseImage = NULL;
	specularImage = NULL;
	normalMap = NULL;
}

//
// PolymerNGMaterialManager
//
class PolymerNGMaterialManager
{
public:
	PolymerNGMaterial		*LoadMaterialForTile(int tileNum);
	PolymerNGMaterial		*LoadMaterial(const char *fileName);

	PolymerNGMaterial		*LoadMaterialFromImage(const char *materialName, BuildImage *image);
private:
	std::vector<PolymerNGMaterial *> materials;
};

extern PolymerNGMaterialManager materialManager;