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

	BuildImage *GetGlowMap()
	{
		return glowMap;
	}

	size_t GetMaterialNameHash()
	{
		return name_hash;
	}

	int GetTileNum()
	{
		return tileNum;
	}
private:
	PolymerNGMaterial(int tileNum);

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

	void	SetGlowMap(BuildImage *image)
	{
		glowMap = image;
	}
private:
	int					tileNum;
	std::string			name;
	size_t				name_hash;

	BuildImage			*diffuseImage;
	BuildImage			*specularImage;
	BuildImage			*normalMap;
	BuildImage			*glowMap;
};

__forceinline PolymerNGMaterial::PolymerNGMaterial(int tileNum)
{
	diffuseImage = NULL;
	specularImage = NULL;
	normalMap = NULL;
	this->tileNum = tileNum;
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