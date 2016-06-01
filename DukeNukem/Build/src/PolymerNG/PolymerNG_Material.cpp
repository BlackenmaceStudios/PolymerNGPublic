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

	PolymerNGMaterial *material = new PolymerNGMaterial();

	BuildImage *diffuseImage = imageManager.LoadTexture(fileName);
	material->SetDiffuseTexture(diffuseImage);
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

	PolymerNGMaterial *material = new PolymerNGMaterial();

	BuildImage *diffuseImage = imageManager.LoadFromTileId(tileNum);
	material->SetDiffuseTexture(diffuseImage);
	material->SetNameAndHash(materialname, materialHash);

	materials.push_back(material);
	return materials[materials.size() - 1];
}