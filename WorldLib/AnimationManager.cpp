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
#include "AnimationManager.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationManager::Init(void)
{
	mcList.Init();
	mcAnimationInvokation.Init(32);
	mcAnimationHappenOnce.Init(128);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationManager::Kill(void)
{
	//The animation happen once data and movement uv data.
	mcAnimationHappenOnce.Kill();
	mcAnimationInvokation.Kill();

	//Kill all the animation index thingies.
	mcList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationManager::SetCelHappenOnceFunction(CAnimation* pcAnimationInvoking, int iInvokingCel, CelFunc Func, float fFractionalTimeToOccur)
{
	SAnimationHappenOnceData*	psData;
	SAnimationCelIndex*			psCel;
	CAnimationIndexCelArray*	pcCelIndices;

	pcCelIndices = pcAnimationInvoking->GetCelIndices();
	psCel = pcCelIndices->SafeGet(iInvokingCel);
    if (psCel)
	{
		psData = mcAnimationHappenOnce.InsertAfterTail();
		psData->fFractionalTimeToOccur = fFractionalTimeToOccur;
		psData->Func = Func;
		psCel->pvData = psData;
		psCel->Func = AnimationHappenOnceFunction;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationManager::SetCelInvokeAnimationFunction(CAnimation* pcAnimationInvoking, int iInvokingCel, int iFlags, CAnimation* pcAnimationToSet, CAnimation** ppcAnimtionPtrToChange, CelFunc Func, float fFractionalTimeToOccur, float fSpeedMultiplier)
{
	SAnimationInvokeAnimationData*	psDataNew;
	SAnimationCelIndex*				psCel;
	CAnimationIndexCelArray*		pcCelIndices;

	pcCelIndices = pcAnimationInvoking->GetCelIndices();
	psCel = pcCelIndices->SafeGet(iInvokingCel);
    if (psCel)
	{
		psDataNew = mcAnimationInvokation.InsertAfterTail();
		psDataNew->fSpeedMultiplier = fSpeedMultiplier;
		psDataNew->iFlags = iFlags;
		psDataNew->pcAnimation = pcAnimationToSet;
		psDataNew->ppcAnimation = ppcAnimtionPtrToChange;
		psDataNew->sAnimationHappenOnceData.fFractionalTimeToOccur = fFractionalTimeToOccur;
		psDataNew->sAnimationHappenOnceData.Func = Func;
		psCel->pvData = psDataNew;
		psCel->Func = AnimationInvokeAnimationFunction;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimation* CAnimationManager::Create(int iNumCels, char* szName)
{
	CAnimation* pcAnimation;

	pcAnimation = mcList.Add();
	if (pcAnimation)
	{
		pcAnimation->Init(iNumCels);
	}
	return pcAnimation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationManager::Release(CAnimation* pcAnimation)
{
	mcList.Remove(pcAnimation);
}

