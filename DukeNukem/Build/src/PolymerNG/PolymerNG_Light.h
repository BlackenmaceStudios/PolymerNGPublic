// PolymerNG_Light.h
//

#pragma once

//
// PolymerNGShadowOccluder
//
struct PolymerNGShadowOccluder
{
	PolymerNGShadowOccluder()
	{
		plane = NULL;
	}

	const Build3DPlane *plane;
	Build3DSprite sprite;
};

//
// PolymerNGShadowPass
// 
struct PolymerNGShadowPass
{
	float4x4 shadowViewProjectionMatrix;
	float4x4 shadowViewMatrix;
	float4x4 shadowProjectionMatrix;
	float frustum[5 * 4];
	float3 spotdir;
	float3 spotRadius;
	std::vector<PolymerNGShadowOccluder> shadowOccluders[2];
};

//
// PolymerNGLightVisbility
//
struct PolymerNGLightVisbility
{
	std::vector<int>		sectorInfluences;
};

//
// PolymerNGLight
//
class PolymerNGLightLocal : public PolymerNGLight
{
public:
	PolymerNGLightLocal(PolymerNGLightOpts opts, PolymerNGBoard *board);

	void		PrepareShadows(Build3DSprite *prsprites, int numSprites, float4x4 modelViewMatrix);

	virtual PolymerNGLightOpts *GetOpts() { return &opts; }
	virtual const PolymerNGLightOpts *GetOriginalOpts() { return &opts_original; }

	const PolymerNGShadowPass *GetShadowPass(int idx) {
		return &shadowPasses[idx];
	}

	BuildRHIMesh *GetRHIMesh()
	{
		return board->GetBaseModel()->rhiVertexBufferStatic;
	}

	bool DoesLightInfluenceSector(int sectorId);

	void  CalculateLightVisibility();
private:
	void  CreateFrustumFromModelViewMatrix(float *modelViewProjectionMatrix, float* frustum);
	bool  IsPlaneInFrustum(Build3DPlane *plane, float* frustum);
	bool  IsPlaneInLight(Build3DPlane* plane);

	PolymerNGLightOpts opts;
	PolymerNGLightOpts opts_original;
	PolymerNGBoard *polymerNGboard;
	Build3DBoard *board;

	PolymerNGLightVisbility lightVisibility;

	float4x4 lightProjectionMatrix;
	PolymerNGShadowPass shadowPasses[6];
};