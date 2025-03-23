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
#include "StandardLib/Unknowns.h"
#include "Movement.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMovement::Init(void)
{
	miType = MT_Unknown;
	mpcRotationAnimation = NULL;
	mpcPositionAnimation = NULL;
	mpcMatrixAnimation = NULL;
	mpsOutput = NULL;
	mpsOffset = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMovement::Kill(void)
{
	mpcRotationAnimation = NULL;
	mpcPositionAnimation = NULL;
	mpcMatrixAnimation = NULL;
	mpsOutput = NULL;
	mpsOffset = NULL;
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMovement::Update(void)
{
	CRotation*			pcRotation;
	CPosition*			pcPosition;
	SFloat3*			psPosition;
	SQuaternion*		psRotation;
	CTransformNode*		pcMatrix;

	if (miType == MT_PRS)
	{
		psRotation = NULL;
		if (mpcRotationAnimation)
		{
			pcRotation = (CRotation*)mpcRotationAnimation->Update();
			psRotation = pcRotation->GetRotation();
		}
		psPosition = NULL;
		if (mpcPositionAnimation)
		{
			pcPosition = (CPosition*)mpcPositionAnimation->Update();
			psPosition = pcPosition->GetPosition();
		}
		D3DXMatrixTransformation((D3DXMATRIX*)&mpsOutput->sD3DMatrix, NULL, NULL, NULL, NULL, (D3DXQUATERNION*)psRotation, (D3DXVECTOR3*)psPosition);
	}
	else if (miType = MT_Matrix)
	{
		pcMatrix = (CTransformNode*)mpcMatrixAnimation->Update();
		mpsOutput->sD3DMatrix = *pcMatrix->GetMatrix();
	}

	if (mpsOffset)
	{
		Float4x4Multiply(&mpsOutput->sD3DMatrix, &mpsOutput->sD3DMatrix, &mpsOffset->sD3DMatrix);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMovement::Set(CAnimation* pcRotationAnimation, CAnimation* pcPositionAnimation, SMatrix* psOutput, SMatrix* psOffset)
{
	miType = MT_PRS;
	mpcMatrixAnimation = NULL;
	mpcRotationAnimation = pcRotationAnimation;
	mpcPositionAnimation = pcPositionAnimation;
	mpsOutput = psOutput;
	mpsOffset = psOffset;
}

void CMovement::Set(CAnimation* pcMatrixAnimation, SMatrix* psOutput, SMatrix* psOffset)
{
	miType = MT_Matrix;
	mpcMatrixAnimation = pcMatrixAnimation;
	mpcRotationAnimation = NULL;
	mpcPositionAnimation = NULL;
	mpsOutput = psOutput;
	mpsOffset = psOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMovement::Restart(void)
{
	if (mpcPositionAnimation)
	{
		mpcPositionAnimation->Restart();
	}
	if (mpcRotationAnimation)
	{
		mpcRotationAnimation->Restart();
	}
	if (mpcMatrixAnimation)
	{
		mpcMatrixAnimation->Restart();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMovement::SetType(int iType)
{
	miType = iType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMovement::SetMatrixAnimation(CAnimation* pcAnimation)
{
	mpcMatrixAnimation = pcAnimation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMovement::SetPositionAnimation(CAnimation* pcAnimation) { mpcPositionAnimation = pcAnimation; }
void CMovement::SetRotationAnimation(CAnimation* pcAnimation) { mpcRotationAnimation = pcAnimation; }
void CMovement::SetOffset(SMatrix* psMatrix) { mpsOffset = psMatrix; }
void CMovement::SetOutput(SMatrix* psMatrix) { mpsOutput = psMatrix; }

