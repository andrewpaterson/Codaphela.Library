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
#include "SceneConverter.h"
#include "AnimationConverter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationConverter::Init(CSceneConverter* pcSceneConverter)
{
	mpcSceneConverter = pcSceneConverter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationConverter::Kill(void)
{
	mpcSceneConverter = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAnimationConverter::Convert(CMovement** ppcMovement, CSequence* pcSequence, SMatrix* psMatrix)
{
	CAnimKeyFrame*					pcAnimKeyFrame;
	CAnimKeyFrame*					pcAnimKeyFramePrev;
	int								i;
	CAnimation*						pcAnimation;
	int								iNumFrames;
	CTransformNodeInterpolator*			psMatrixInterpolator;
	CMovement*						pcMovement;
	CTransformNode*						pcMatrix;

	//Something, Something, Something...  IsAnimationConverted() ... blah...

	pcMovement = mpcSceneConverter->GetWorld()->CreateMovement();

	pcAnimation = NULL;
	pcAnimKeyFramePrev = NULL;
	iNumFrames = pcSequence->masKeyFrames.NumElements();
	for (i = 0; i < iNumFrames; i++)
	{
		pcAnimKeyFrame = pcSequence->masKeyFrames.Get(i);
		if (pcAnimKeyFrame->miType & KFT_Matrix)
		{
			//If we don't yet have an animation then create one.
			if (pcAnimation == NULL)
			{
				pcAnimation = mpcSceneConverter->GetWorld()->CreateAnimation(iNumFrames/3 + 1);
			}

			//Only if we already have a position key can we create a position animation.
			if (pcAnimKeyFramePrev)
			{
				//This is because the start and end points are needed by pcPosition.
				pcMatrix = mpcSceneConverter->GetWorld()->CreateAnimMatrix();
				psMatrixInterpolator = (CTransformNodeInterpolator*)pcMatrix->Set(MT_SphericalLinearMatrix);
				pcAnimation->Add(pcMatrix, pcAnimKeyFrame->mfTime, -1, MatrixFunction, NULL);
				psMatrixInterpolator->msPosition1 = ((SAnimKeyFrameMatrix*)(&pcAnimKeyFramePrev->msf))->msMatrix;
				psMatrixInterpolator->msPosition2 = ((SAnimKeyFrameMatrix*)(&pcAnimKeyFrame->msf))->msMatrix;
				psMatrixInterpolator->mfExponential = 1.0f;
			}

			pcAnimKeyFramePrev = pcAnimKeyFrame;
		}
	}

	if (pcAnimation)
	{
		pcAnimation->MakeLoopingAnimation();
	}

	pcMovement->SetType(MT_Matrix);
	pcMovement->SetMatrixAnimation(pcAnimation);
	pcMovement->SetPositionAnimation(NULL);
	pcMovement->SetRotationAnimation(NULL);
	pcMovement->SetOffset(NULL);
	pcMovement->SetOutput(psMatrix);
	pcMovement->Restart();
	pcMovement->Update();

	if (ppcMovement)
	{
		*ppcMovement = pcMovement;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAnimationConverter::IsAnimationConverted(void)
{
	//I should probably fix this at some point...
	return false;
}