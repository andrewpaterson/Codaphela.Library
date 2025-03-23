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
#include "AnimationInvokeAnimation.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AnimationInvokeAnimationFunction(void* pvCel, float fFractionalTime, void* pvData)
{
	SAnimationInvokeAnimationData*	psAnimData;
	SAnimationCelIndex*				psCel;

	psCel = (SAnimationCelIndex*)pvCel;
	psAnimData = (SAnimationInvokeAnimationData*)pvData;
	if (psCel->iFlags & AHOF_FirstTick) 
	{
		if (psAnimData->iFlags & AIAF_ResetAnimtionOnFirstFrame)
		{
			psAnimData->pcAnimation->Restart();
		}
		if (psAnimData->iFlags & AIAF_SetSpeedMultipler)
		{
			psAnimData->pcAnimation->SetSpeed(psAnimData->fSpeedMultiplier);
		}
		(*psAnimData->ppcAnimation) = psAnimData->pcAnimation;
	}

	AnimationHappenOnceFunction(psCel, fFractionalTime, pvData);
}

