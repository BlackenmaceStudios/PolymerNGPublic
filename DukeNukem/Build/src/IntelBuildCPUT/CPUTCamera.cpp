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
#include "CPUTCamera.h"
#include "CPUTFrustum.h"

// Constructor
//-----------------------------------------------------------------------------
CPUTCamera::CPUTCamera() : 
    mFov(45.0f * 3.14159265f/180.0f),
    mNearPlaneDistance(1.0f),
    mFarPlaneDistance(100.0f),
    mAspectRatio(16.0f/9.0f)
{

}


//-----------------------------------------------------------------------------
void CPUTCamera::SetAspectRatio(const float aspectRatio)
{
    mAspectRatio = aspectRatio;
}

//-----------------------------------------------------------------------------
void CPUTCamera::SetFov(const float fov)
{
    mFov = fov;
}

CPUTCamera &CPUTCamera::operator=(const CPUTCamera& camera)
{
	mFov = camera.mFov;
	mNearPlaneDistance = camera.mNearPlaneDistance;
	mFarPlaneDistance = camera.mFarPlaneDistance;
	mAspectRatio = camera.mAspectRatio;
	mView = camera.mView;
	mProjection = camera.mProjection;

	for(int i = 0; i < 8; i++)
	{
		mFrustum.mpPosition[i] = camera.mFrustum.mpPosition[i]; 
	}

	for(int i = 0; i < 6; i++)
	{
		mFrustum.mpNormal[i] = camera.mFrustum.mpNormal[i];
		mFrustum.mPlanes[0*8 + i] = mFrustum.mpNormal[i].x;
		mFrustum.mPlanes[1*8 + i] = mFrustum.mpNormal[i].y;
		mFrustum.mPlanes[2*8 + i] = mFrustum.mpNormal[i].z;
		mFrustum.mPlanes[3*8 + i] = -dot3(mFrustum.mpNormal[i], mFrustum.mpPosition[(i < 3) ? 0 : 6]);
	}

	for (int i=6; i < 8; i++)
	{
		mFrustum.mPlanes[0*8 + i] = 0;
		mFrustum.mPlanes[1*8 + i] = 0;
		mFrustum.mPlanes[2*8 + i] = 0;
		mFrustum.mPlanes[3*8 + i] = -1.0f;
	}

	return *this;
}