/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/FloatHelper.h"
#include "BaseLib/Numbers.h"
#include "Triangle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangle::Init(SFloat3* psPoint1, SFloat3* psPoint2, SFloat3* psPoint3, SFloat3* psNormal)
{
	CPlane::Init(psPoint1, psNormal);
	mpsPosition1 = psPoint2;
	mpsPosition2 = psPoint3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangle::Set(void)
{
	CPlane::Set();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangle::Set(SFloat3* psPoint1, SFloat3* psPoint2, SFloat3* psPoint3)
{
	CPlane::Set(psPoint1, psPoint2, psPoint3);
	*mpsPosition1 = *psPoint2;
	*mpsPosition2 = *psPoint3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangle::Set(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
{
	CPlane::Set(x1, y1, z1, x2, y2, z1, x3, y3, z3);
	mpsPosition1->Init(x2, y2, z2);
	mpsPosition2->Init(x3, y3, z3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CTriangle::GetPoint(int iPoint)
{
	if (iPoint == 0)
	{
		return mpsPosition;
	}
	else if (iPoint == 1)
	{
		return mpsPosition1;
	}
	else if (iPoint == 2)
	{
		return mpsPosition2;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTriangle::HasPositionPtr(SFloat3* psPosition)
{
	if (mpsPosition == psPosition)
	{
		return true;
	}
	if (mpsPosition1 == psPosition)
	{
		return true;
	}
	if (mpsPosition2 == psPosition)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangle::Center(SFloat3* psCenter)
{
	*psCenter = *mpsPosition;
	*psCenter += *mpsPosition1;
	*psCenter += *mpsPosition2;

	*psCenter /= 3.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTriangle::IsDegenerate(void)
{
	if ((mpsPosition == mpsPosition1) || (mpsPosition == mpsPosition2) || (mpsPosition1 == mpsPosition2))
	{
		return true;
	}
	
	if ((mpsPosition->CloselyEqual(mpsPosition1) || mpsPosition->CloselyEqual(mpsPosition2) || mpsPosition1->CloselyEqual(mpsPosition2)))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangle::GetEdgeDirections(SFloat3* psEdge1, SFloat3* psEdge2, SFloat3* psEdge3)
{
	*psEdge1 = *mpsPosition1 - *mpsPosition;
	*psEdge2 = *mpsPosition2 - *mpsPosition1;
	*psEdge3 = *mpsPosition - *mpsPosition2;

	psEdge1->Normalize();
	psEdge2->Normalize();
	psEdge3->Normalize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangle::GetAngles(float* psAngle0, float* psAngle1, float* psAngle2)
{
	SFloat3	sSide1;
	SFloat3	sSide2;
	SFloat3	sSide3;
	SFloat3 sCross;
	float   fDot;

	GetEdgeDirections(&sSide1, &sSide2, &sSide3);

	Float3Cross(&sCross, &sSide3, &sSide1);
	fDot = Float3Dot(&sSide3, &sSide1);
	*psAngle0 = asinf(sCross.Magnitude());
	if (fDot > 0.0f)
		*psAngle0 = N_PI - *psAngle0;
	Float3Cross(&sCross, &sSide1, &sSide2);
	fDot = Float3Dot(&sSide1, &sSide2);
	*psAngle1 = asinf(sCross.Magnitude());
	if (fDot > 0.0f)
		*psAngle1 = N_PI - *psAngle1;
	Float3Cross(&sCross, &sSide2, &sSide3);
	fDot = Float3Dot(&sSide2, &sSide3);
	*psAngle2 = asinf(sCross.Magnitude());
	if (fDot > 0.0f)
		*psAngle2 = N_PI - *psAngle2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTriangle::IsSliver(float fAngleSliver, SFloat3** pps0, SFloat3** pps1, SFloat3** pps2)
{
	float	fAngle0;
	float	fAngle1;
	float	fAngle2;
	int		iIndex0;
	int		iIndex1;
	int		iIndex2;

	GetAngles(&fAngle0, &fAngle1, &fAngle2);
	iIndex0 = 0;
	iIndex1 = 1;
	iIndex2 = 2;
	if (fAngle0 < fAngle2)
	{
		Swap(&fAngle0, &fAngle2);
		Swap(&iIndex0, &iIndex2);
	}
	if (fAngle0 < fAngle1)
	{
		Swap(&fAngle0, &fAngle1);
		Swap(&iIndex0, &iIndex1);
	}
	if (fAngle1 < fAngle2)
	{
		Swap(&fAngle1, &fAngle2);
		Swap(&iIndex1, &iIndex2);
	}

	*pps0 = GetPoint(iIndex0);
	*pps1 = GetPoint(iIndex1);
	*pps2 = GetPoint(iIndex2);

	if (fAngle0 <= fAngleSliver)
	{
		gcUserError.Set("How does a triangle with three *tiny* angles exist?");
		return 3;
	}

	if (fAngle1 <= fAngleSliver)
	{
		return 2;
	}
	if (fAngle2 <= fAngleSliver)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTriangle::Dump(void)
{
	CChars	sz;

	sz.Init();
	sz.Append("--------------------\n");
	mpsNormal->Print(&sz, 4);
	sz.AppendNewLine();
	sz.AppendNewLine();
	mpsPosition->Print(&sz, 4);
	sz.AppendNewLine();
	mpsPosition1->Print(&sz, 4);
	sz.AppendNewLine();
	mpsPosition2->Print(&sz, 4);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

