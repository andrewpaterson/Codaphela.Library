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
#ifndef __ANIMATION_INVOKE__ANIMATION_H__
#define __ANIMATION_INVOKE__ANIMATION_H__
#include "Animation.h"


enum EAnimationInvokeAnimationFlags
{
	AIAF_ResetAnimtionOnFirstFrame = 1,
	AIAF_SetSpeedMultipler = 2,
};


struct SAnimationInvokeAnimationData
{
	SAnimationHappenOnceData	sAnimationHappenOnceData;
	CAnimation**				ppcAnimation;  //The pointer to change...
	CAnimation*					pcAnimation;  //and the animation to change it to.
	float						fSpeedMultiplier;
	int							iFlags;
};


//This assumes that pvData is already a filled in 'SAnimationInvokeAnimationData'.
void AnimationInvokeAnimationFunction(void* psCel, float fFractionalTime, void* pvData);


#endif //__ANIMATION_INVOKE__ANIMATION_H__

