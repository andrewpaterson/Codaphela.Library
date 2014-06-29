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
#include "Rotation.h"
#include "BaseLib/Numbers.h"


//////////////////////////////////////////////////////////////////////////
//	Rotation Animation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRotationInterpolator::Init(void)
{
	PrivateInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRotationInterpolator::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRotationInterpolator::Interpolate(float fPos, SQuaternion* psRotation1, SQuaternion* psRotation2)
{
	float	fInv;

	fInv = 1.0f - fPos;
	D3DXQuaternionSlerp((D3DXQUATERNION*)&msOutput, (D3DXQUATERNION*)psRotation1, (D3DXQUATERNION*)psRotation2, fInv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRotationInterpolator::Update(float fPos)
{
	Interpolate(1.0f - fPos, &msPosition1, &msPosition2);
}


//////////////////////////////////////////////////////////////////////////
//	Rotation Animation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRotation::Init(void)
{
	msType.iValue = -1;
	msType.pvData = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRotation::Kill(void)
{
	if (msType.pvData != NULL)
	{
		free(msType.pvData);
		msType.pvData = NULL;
	}
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRotation::Set(ERotationType eType)
{
	CRotationInterpolator*	psLinear;

	if (eType == RT_SphericalLinearRotation)
	{
		msType.pvData = malloc(sizeof(CRotationInterpolator));
		psLinear = (CRotationInterpolator*)msType.pvData;
		msType.iValue = eType;
		psLinear->Init();
		return psLinear;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRotation::Update(float fFractionalTime)
{
	switch(msType.iValue) 
	{
	case RT_SphericalLinearRotation:
		((CRotationInterpolator*)msType.pvData)->Update(fFractionalTime);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuaternion* CRotation::GetRotation(void)
{
	switch(msType.iValue) 
	{
	case RT_SphericalLinearRotation:
		return &(((CRotationInterpolator*)msType.pvData)->msOutput);
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void RotationFunction(void* pvCel, float fFractionalTime, void* pvData)
{
	//This is the rotation function which an animation can play.
	CRotation*	pcRotation;

	pcRotation = (CRotation*)pvCel;
	pcRotation->Update(fFractionalTime);
}

