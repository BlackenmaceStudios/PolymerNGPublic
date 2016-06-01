//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#ifndef __CPUTCamera_H__
#define __CPUTCamera_H__

#include <memory.h>
#include <Windows.h>
#include "CPUT.h"
#include "CPUTFrustum.h"

//-----------------------------------------------------------------------------
class CPUTCamera
{
public:
    float mFov;                // the field of view in degrees
    float mNearPlaneDistance;  // near plane distance
    float mFarPlaneDistance;   // far plane distance
    float mAspectRatio;        // width/height.  TODO: Support separate pixel and viewport aspect ratios
protected:
    float4x4 mView;
    float4x4 mProjection;

public:
    CPUTFrustum mFrustum;

    CPUTCamera();
    ~CPUTCamera() {}

    void Update( float deltaSeconds=0.0f ) {
        mFrustum.InitializeFrustum(this);
    };


    float4x4 *GetViewMatrix(void)
    {
        // Update();  We can't afford to do this every time we're asked for the view matrix.  Caller needs to make sure camera is updated before entering render loop.
        return &mView;
    }

    const float4x4* GetProjectionMatrix(void) const { return &mProjection; }
    void            SetProjectionMatrix(const float4x4 &projection) { mProjection = projection; }
	void			SetViewMatrix(const float4x4 &viewMatrix) { mView = viewMatrix; }
    float           GetAspectRatio() { return mAspectRatio; }
    float           GetFov() { return mFov; }
    void            SetAspectRatio(const float aspectRatio);
    void            SetFov( const float fov );
    float           GetNearPlaneDistance() { return mNearPlaneDistance; }
    float           GetFarPlaneDistance() {  return mFarPlaneDistance; }
    void            SetNearPlaneDistance( const float nearPlaneDistance ) { mNearPlaneDistance = nearPlaneDistance; }
    void            SetFarPlaneDistance(  const float farPlaneDistance ) { mFarPlaneDistance = farPlaneDistance; }

	void GetPosition(float3 *pPosition)
	{
		pPosition->x = mView.r3.x;
		pPosition->y = mView.r3.y;
		pPosition->z = mView.r3.z;
	}

	float3 GetPosition()
	{
		float3 ret = float3(mView.r3.x, mView.r3.y, mView.r3.z);
		return ret;
	}

	float3 GetLook()
	{
		return mView.getZAxis();
	}
	float3 GetUp()
	{
		return mView.getYAxis();
	}
	float3 GetLook(float *pX, float *pY, float *pZ)
	{
		float3 look = mView.getZAxis();
		*pX = look.x;
		*pY = look.y;
		*pZ = look.z;
	}

	CPUTCamera &operator=(const CPUTCamera& camera);
};

#endif //#ifndef __CPUTCamera_H__
