/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "TransformNode.h"
#include "BaseLib/Numbers.h"


//////////////////////////////////////////////////////////////////////////
//	Matrix Animation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeInterpolator::Init(void)
{
	PrivateInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeInterpolator::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeInterpolator::Interpolate(float fPos, SFloat4x4* psMatrix1, SFloat4x4* psMatrix2)
{
	float	fInv;

	fInv = 1.0f - fPos;

	//Um, you need to actually interpolate here.
	msOutput = *psMatrix1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNodeInterpolator::Update(float fPos)
{
	Interpolate(1.0f - fPos, &msPosition1, &msPosition2);
}


//////////////////////////////////////////////////////////////////////////
//	Matrix Animation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNode::Init(void)
{
	msType.iValue = -1;
	msType.pvData = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransformNode::Kill(void)
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
void* CTransformNode::Set(EMatrixType eType)
{
	CTransformNodeInterpolator*	psLinear;

	if (eType == MT_SphericalLinearMatrix)
	{
		msType.pvData = malloc(sizeof(CTransformNodeInterpolator));
		psLinear = (CTransformNodeInterpolator*)msType.pvData;
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
void CTransformNode::Update(float fFractionalTime)
{
	switch(msType.iValue) 
	{
	case MT_SphericalLinearMatrix:
		((CTransformNodeInterpolator*)msType.pvData)->Update(fFractionalTime);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat4x4* CTransformNode::GetMatrix(void)
{
	switch(msType.iValue) 
	{
	case MT_SphericalLinearMatrix:
		return &(((CTransformNodeInterpolator*)msType.pvData)->msOutput);
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MatrixFunction(void* pvCel, float fFractionalTime, void* pvData)
{
	//This is the Matrix function which an animation can play.
	CTransformNode*	pcMatrix;

	pcMatrix = (CTransformNode*)pvCel;
	pcMatrix->Update(fFractionalTime);
}

