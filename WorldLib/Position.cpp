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
#include "Position.h"
#include "BaseLib/Numbers.h"


//////////////////////////////////////////////////////////////////////////
//	Position Animation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositionInterpolator::Interpolate(float fPos, SFloat3* psPosition1, SFloat3* psPosition2)
{
	float	fInv;

	fInv = 1.0f - fPos;

	msOutput = *psPosition1;
	msOutput.x *= fPos;
	msOutput.y *= fPos;
	msOutput.z *= fPos;

	msOutput.x += psPosition2->x * fInv;
	msOutput.y += psPosition2->y * fInv;
	msOutput.z += psPosition2->z * fInv;
}


//////////////////////////////////////////////////////////////////////////
//	Linear Position
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinearPositionInterpolator::Init(void)
{
	PrivateInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinearPositionInterpolator::Update(float fPos)
{
	fPos = GetExponentialAdjustedPos(fPos);
	Interpolate(1.0f - fPos, &msPosition1, &msPosition2);
}


//////////////////////////////////////////////////////////////////////////
//	Directed Position
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectedInterpolator::Init(void)
{
	PrivateInit();
	msDir1 = msDir2 = SFloat3(0,0,0);
	mfDir1Length = 1.0f;
	mfDir2Length = -1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectedInterpolator::Update(float fPos)
{
	SFloat3		sNewPos1;
	SFloat3		sNewPos2;
	float			fInv;

	fPos = GetExponentialAdjustedPos(fPos);
	fInv = 1.0f - fPos;

	sNewPos1 = msPosition1 + (msDir1 * mfDir1Length * fPos);
	sNewPos2 = msPosition2 + (msDir2 * mfDir2Length * fInv);

	Interpolate(fInv, &sNewPos1, &sNewPos2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectedInterpolator::SetDirections(SFloat3* psStart, SFloat3* psEnd)
{
	Float3Normalize(&msDir1, psStart);
	Float3Normalize(&msDir2, psEnd);
	mfDir1Length = Float3Length(psStart);
	mfDir2Length = Float3Length(psEnd);
}


//////////////////////////////////////////////////////////////////////////
//	Position Animation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPosition::Init(void)
{
	msType.iType = -1;
	msType.pvData = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPosition::Kill(void)
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
void* CPosition::Set(EPositionType eType)
{
	CLinearPositionInterpolator*	psLinear;
	CDirectedInterpolator*			psDirected;

	if (eType == PT_LinearPosition)
	{
		//Why the fuck do you malloc this?!??!
		msType.pvData = malloc(sizeof(CLinearPositionInterpolator));
		psLinear = (CLinearPositionInterpolator*)msType.pvData;
		msType.iType = eType;
		psLinear->Init();
		return psLinear;
	}
	else if (eType == PT_DirectedPosition)
	{
		msType.pvData = malloc(sizeof(CDirectedInterpolator));
		psDirected = (CDirectedInterpolator*)msType.pvData;
		msType.iType = eType;
		psDirected->Init();
		return psDirected;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPosition::Update(float fFractionalTime)
{
	switch(msType.iType) 
	{
	case PT_LinearPosition:
		((CLinearPositionInterpolator*)msType.pvData)->Update(fFractionalTime);
		break;
	case PT_DirectedPosition:
		((CDirectedInterpolator*)msType.pvData)->Update(fFractionalTime);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CPosition::GetPosition(void)
{
	switch(msType.iType) 
	{
	case PT_LinearPosition:
	case PT_DirectedPosition:
		return &(((CPositionInterpolator*)msType.pvData)->msOutput);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PositionFunction(void* pvCel, float fFractionalTime, void* pvData)
{
	//This is the rotation function which an animation can play.
	CPosition*	pcPosition;

	pcPosition = (CPosition*)pvCel;
	pcPosition->Update(fFractionalTime);
}

