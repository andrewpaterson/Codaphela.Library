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
#ifndef __ANIMATION_MANAGER_H__
#define __ANIMATION_MANAGER_H__
#include "BaseLib/LinkListTemplateFreeList.h"
#include "StandardLib/SetType.h"
#include "Animation.h"
#include "AnimationInvokeAnimation.h"


typedef CSetType<CAnimation>	CAnimationList;
typedef CLinkListTemplateFreeList<SAnimationInvokeAnimationData>	CLinkListAnimationInvokeAnimationData;
typedef CLinkListTemplateFreeList<SAnimationHappenOnceData>			CLinkListAnimationHappenOnceData;


class CAnimationManager
{
public:
	CAnimationList							mcList;
	CLinkListAnimationHappenOnceData		mcAnimationHappenOnce;
	CLinkListAnimationInvokeAnimationData	mcAnimationInvokation;

	void					Init(void);
	void					Kill(void);

	void					SetCelHappenOnceFunction(CAnimation* pcAnimationInvoking, int iInvokingCel, CelFunc Func, float fFractionalTimeToOccur);
	void					SetCelInvokeAnimationFunction(CAnimation* pcAnimationInvoking, int iInvokingCel, int iFlags, CAnimation* pcAnimationToSet, CAnimation** ppcAnimtionPtrToChange, CelFunc Func, float fFractionalTimeToOccur, float fSpeedMultiplier = 1.0f);
	CAnimation*				Create(int iNumCels, char* szName = NULL);
	void					Release(CAnimation* pcAnimation);
};


#endif //__ANIMATION_MANAGER_H__

