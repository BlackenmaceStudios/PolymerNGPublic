// CacheModel.h
//

#pragma once

class PolymerNGModelCachePayload;
class BaseModel;
class PolymerNGMaterial;
class PolymerNGModelCacheSurfaceDefine;

struct CacheModelSurface
{
	PolymerNGMaterial *material;
	int startVertex;
	int numVertexes;

	int startIndex;
	int numIndexes;
};

//
// CacheModel
//
class CacheModel 
{
public:
	void			Init(const PolymerNGModelCachePayload *payload);

	void			SetTextureForSurface(PolymerNGModelCacheSurfaceDefine *cacheSurfaceDefines);

	void			BindToBaseMesh(BaseModel *mesh);

	void			Unload();
public:
	int				GetNumSurfaces();

	CacheModelSurface *GetCacheSurface(int idx) { return &surfaces[idx]; }

	// Functions needed for rendering.
	BaseModel		*GetBaseModel() { return baseMesh; }
private:
	const PolymerNGModelCachePayload *payload;
	CacheModelSurface	surfaces[20];

	BaseModel		*baseMesh;
};
