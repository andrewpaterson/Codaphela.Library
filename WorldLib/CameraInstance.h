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
#ifndef __CAMERA_INSTANCE_H__
#define __CAMERA_INSTANCE_H__
#include "DirectXLib/Direct3DGlobal.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Ray.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/StandardHeader.h"


class CWorld;
class CCameraInstance : public CUnknown
{
CONSTRUCTABLE(CCameraInstance);
protected:
	SSurface*	mpsSurface;
	SMatrix*	mpsView;  //This will transform the view about the camera (practically keeping the camera at 0,0,0).
						  //This is the matrix used for the view transform (not including perpective).

	SMatrix*	mpsProjection;
	SMatrix*	mpsWorld;  //This will transform the camera into world space
						   //This is used to position the camera and calculate the view matrix from it.

	SMatrix*	mpsAdjustment;  //A final transform (rotation) to be applied before the camera is used.

	float		mfFOVVertical;
	float		mfAspectRatio;
	float		mfNearPlane;
	float		mfFarPlane;

	CWorld*		mpcWorld;
	//Must still add user clip planes.

public:
	void 		Init(CWorld* pcWorld);
	void 		Kill(void);
	void 		Set(float fFOVVertical, float fAspectRatio, float fNearPlane, float fFarPlane);
	void 		SetSurface(SSurface* psSurface);
	void 		SetWorldMatrix(SMatrix* psMatrix);
	void 		SetFOV(float fVertical, float fAspectRatio);
	void 		SetNearAndFarPlanes(float fNearPlane, float fFarPlane);
	bool 		Use(void);
	void 		Adjust(SFloat4x4* psDest);
	void 		LookAt(SFloat3* psFrom, SFloat3* psTo, SFloat3* psUp);
	void 		CalculateView(void);
	void 		CalculateProjection(void);
	bool 		IsSphereVisible(CSphere* pcSphere);
	void 		PickRay(CRay* pcRay, SFloat2 sCursorPosition);
	void 		CreateAdjustment(void);
	SMatrix*	GetViewTransform(void);
	SMatrix*	GetWorldTransform(void);
	SMatrix*	GetAdjustmentTransform(void);
	SMatrix*	GetProjectionTransform(void);
	void 		SetViewTransform(SMatrix* psMatrix);
	void 		SetWorldTransform(SMatrix* psMatrix);
	void 		SetAdjustmentTransform(SMatrix* psMatrix);
	void 		SetProjectionTransform(SMatrix* psMatrix);
	float		GetFOVVertical(void);
	float		GetAspectRatio(void);
	float		GetNearPlane(void);
	float		GetFarPlane(void);
};


#endif //__CAMERA_INSTANCE_H__



