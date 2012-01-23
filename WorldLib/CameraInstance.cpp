/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "CameraInstance.h"
#include "World.h"
#include "SupportLib/Ray.h"
#include "BaseLib/Numbers.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::Init(CWorld* pcWorld)
{
	mpsSurface = NULL;
	mpsWorld = NULL;
	mpsAdjustment = NULL;
	mpcWorld = pcWorld;
	mpsProjection = mpcWorld->CreateMatrix();
	mpsView = mpcWorld->CreateMatrix();

	Set(45.0f, 1.3333333f, 0.2f, 1000.0f);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::Set(float fFOVVertical, float fAspectRatio, float fNearPlane, float fFarPlane)
{
	mfFOVVertical = fFOVVertical;
	mfAspectRatio = fAspectRatio;
	mfNearPlane = fNearPlane;
	mfFarPlane = fFarPlane;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::Kill(void)
{
	mpcWorld->ReleaseMatrix(mpsView);
	mpcWorld->ReleaseMatrix(mpsWorld);
	if (mpsAdjustment)
	{
		mpcWorld->ReleaseMatrix(mpsAdjustment);
	}
	if (mpsProjection)
	{
		mpcWorld->ReleaseMatrix(mpsProjection);
	}
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::SetSurface(SSurface* psSurface)
{
	mpsSurface = psSurface;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::SetWorldMatrix(SMatrix* psMatrix)
{
	mpsWorld = psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::SetFOV(float fVertical, float fAspectRatio)
{
	mfFOVVertical = fVertical;
	mfAspectRatio = fAspectRatio;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::SetNearAndFarPlanes(float fNearPlane, float fFarPlane)
{
	mfNearPlane = fNearPlane;
	mfFarPlane = fFarPlane;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCameraInstance::Use(void)
{
	//It's allowable to have a NULL surface... the render target won't be changed.
	if (mpsSurface != NULL)
	{
		gcD3D.SetRenderTarget(mpsSurface);
	}

	//Do this every frame... not a problem on todays hardware.
	CalculateView();
	CalculateProjection();

	return gcD3D.SetCamera(mpsProjection, mpsView);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::LookAt(SFloat3* psFrom, SFloat3* psTo, SFloat3* psUp)
{
	Float4x4LookAtRH(&mpsView->sD3DMatrix, psFrom, psTo, psUp);
	Float4x4Inverse(&mpsWorld->sD3DMatrix, NULL, &mpsView->sD3DMatrix);

	//Remove this?
	CalculateView();
	CalculateProjection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::CalculateProjection(void)
{
	Float4x4PerspectiveFovRH((SFloat4x4*)mpsProjection, Deg2Rad(mfFOVVertical), mfAspectRatio, mfNearPlane, mfFarPlane);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::CalculateView(void)
{
	if (!mpsAdjustment)
	{
		Float4x4Inverse(&mpsView->sD3DMatrix, NULL, &mpsWorld->sD3DMatrix);
	}
	else
	{
		Float4x4Multiply(&mpsView->sD3DMatrix, &mpsAdjustment->sD3DMatrix, &mpsWorld->sD3DMatrix);
		Float4x4Inverse(&mpsView->sD3DMatrix, NULL, &mpsView->sD3DMatrix);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::Adjust(SFloat4x4* psDest)
{
	if (!mpsAdjustment)
	{
		*psDest = mpsWorld->sD3DMatrix;
	}
	else
	{
		Float4x4Multiply(psDest, &mpsAdjustment->sD3DMatrix, &mpsWorld->sD3DMatrix);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::CreateAdjustment(void)
{
	if (!mpsAdjustment)
	{
		mpsAdjustment = mpcWorld->CreateMatrix();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCameraInstance::IsSphereVisible(CSphere* pcSphere)
{
	//Naive implementation.
	return TRUE;  

	//Cone = mcShapeTrans.GetFirstCone()

	//	Test cone against sphere.

	//Convex Hull = mcShapeTrans.GetFirstConvexHull();

	//	Test convex hull against sphere.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::PickRay(CRay* pcRay, SFloat2 sWindowPosition)
{
	SFloat3		sAt;
	SFloat3		sUp;
	SFloat3		sAcross;

	//You may be using the wrong matrix here... test it.
	Float3Assign((SFloat3*)&pcRay->msStart, (SFloat3*)mpsWorld->Pos());

	Float3Assign(&sAt, mpsWorld->At());
	Float3Scale(&sUp, mpsWorld->Up(), sWindowPosition.y);
	Float3Scale(&sAcross, mpsWorld->Across(), sWindowPosition.x);
	Float3Add(&sAt, &sAt, &sUp);
	Float3Add(&sAt, &sAt, &sAcross);
	Float3Normalize((SFloat3*)&pcRay->msDirection, &sAt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CCameraInstance::GetViewTransform(void)
{
	return mpsView;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CCameraInstance::GetWorldTransform(void)
{
	return mpsWorld;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CCameraInstance::GetAdjustmentTransform(void)
{
	return mpsAdjustment;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CCameraInstance::GetProjectionTransform(void)
{
	return mpsProjection;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::SetViewTransform(SMatrix* psMatrix)
{
	mpsView = psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::SetWorldTransform(SMatrix* psMatrix)
{
	mpsWorld = psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::SetAdjustmentTransform(SMatrix* psMatrix)
{
	mpsAdjustment = psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCameraInstance::SetProjectionTransform(SMatrix* psMatrix)
{
	mpsProjection = psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CCameraInstance::GetFOVVertical(void)
{
	return mfFOVVertical;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CCameraInstance::GetAspectRatio(void)
{
	return mfAspectRatio;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CCameraInstance::GetNearPlane(void)
{
	return mfNearPlane;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CCameraInstance::GetFarPlane(void)
{
	return mfFarPlane;
}

