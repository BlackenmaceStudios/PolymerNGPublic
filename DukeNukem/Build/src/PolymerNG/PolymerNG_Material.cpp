// PolymerNG_Material.cpp
//

#include "PolymerNG_local.h"

PolymerNGMaterialManager materialManager;

//
// PolymerNGMaterialManager::LoadMaterial
//
PolymerNGMaterial * PolymerNGMaterialManager::LoadMaterial(const char *fileName)
{
	char materialname[256];
	size_t materialHash;

	sprintf(materialname, "hires/%s", fileName);

	materialHash = std::hash<std::string>()(materialname);

	// Check to see if we have this material is already loaded.
	int numLoadedMaterials = materials.size();
	PolymerNGMaterial **loadedMaterials = NULL;

	if (numLoadedMaterials > 0)
	{
		loadedMaterials = &materials[0];
	}
	for (int i = 0; i < numLoadedMaterials; i++)
	{
		if (loadedMaterials[i]->GetMaterialNameHash() == materialHash)
			return loadedMaterials[i];
	}

	PolymerNGMaterial *material = new PolymerNGMaterial(-1);

	BuildImage *diffuseImage = imageManager.LoadTexture(fileName);
	material->SetDiffuseTexture(diffuseImage);
	material->SetNameAndHash(materialname, materialHash);

	materials.push_back(material);
	return materials[materials.size() - 1];
}
//
// PolymerNGMaterialManager::LoadMaterialFromImage
//
PolymerNGMaterial * PolymerNGMaterialManager::LoadMaterialFromImage(const char *materialName, BuildImage *image)
{
	char materialname[256];
	size_t materialHash;

	sprintf(materialname, "engine/memory/%s", materialName);

	materialHash = std::hash<std::string>()(materialname);

	// Check to see if we have this material is already loaded.
	int numLoadedMaterials = materials.size();
	PolymerNGMaterial **loadedMaterials = NULL;

	if (numLoadedMaterials > 0)
	{
		loadedMaterials = &materials[0];
	}
	for (int i = 0; i < numLoadedMaterials; i++)
	{
		if (loadedMaterials[i]->GetMaterialNameHash() == materialHash)
			return loadedMaterials[i];
	}

	PolymerNGMaterial *material = new PolymerNGMaterial(-1);
	material->SetDiffuseTexture(image);

	material->SetNameAndHash(materialname, materialHash);

	materials.push_back(material);
	return materials[materials.size() - 1];
}

//
// PolymerNGMaterialManager::LoadMaterialForTile
//
PolymerNGMaterial *PolymerNGMaterialManager::LoadMaterialForTile(int tileNum)
{
	char materialname[256];
	size_t materialHash;

	sprintf(materialname, "engine/tiles/%d", tileNum);

	materialHash = std::hash<std::string>()(materialname);

	// Check to see if we have this material is already loaded.
	int numLoadedMaterials = materials.size();
	PolymerNGMaterial **loadedMaterials = NULL;
	
	if (numLoadedMaterials > 0)
	{
		loadedMaterials = &materials[0];
	}
	for (int i = 0; i < numLoadedMaterials; i++)
	{
		if (loadedMaterials[i]->GetMaterialNameHash() == materialHash)
			return loadedMaterials[i];
	}

	PolymerNGMaterial *material = new PolymerNGMaterial(tileNum);

	BuildImage *diffuseImage = imageManager.LoadFromTileId(tileNum, PAYLOAD_IMAGE_DIFFUSE);
	material->SetDiffuseTexture(diffuseImage);

	BuildImage *normalImage = imageManager.LoadFromTileId(tileNum, PAYLOAD_IMAGE_NORMAL);
	material->SetNormalMap(normalImage);

	BuildImage *specularImage = imageManager.LoadFromTileId(tileNum, PAYLOAD_IMAGE_SPECULAR);
	material->SetSpecularMap(specularImage);

	BuildImage *glowImage = imageManager.LoadFromTileId(tileNum, PAYLOAD_IMAGE_GLOW);
	material->SetGlowMap(glowImage);

	material->SetNameAndHash(materialname, materialHash);

	materials.push_back(material);
	return materials[materials.size() - 1];
}