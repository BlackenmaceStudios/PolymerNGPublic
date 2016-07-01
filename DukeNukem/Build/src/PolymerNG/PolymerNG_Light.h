// PolymerNG_Light.h
//

#pragma once

//
// PolymerNGShadowOccluder
//
struct PolymerNGShadowOccluder
{
	const Build3DPlane *plane;
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
	PolymerNGLightLocal(PolymerNGLightOpts opts, Build3DBoard *board);

	void		PrepareShadows();

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

	PolymerNGLightOpts opts;
	PolymerNGLightOpts opts_original;
	Build3DBoard *board;

	PolymerNGLightVisbility lightVisibility;

	float4x4 lightProjectionMatrix;
	PolymerNGShadowPass shadowPasses[6];
};