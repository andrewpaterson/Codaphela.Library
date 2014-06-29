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
#ifndef __ANIMATION_H__
#define __ANIMATION_H__
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/StandardHeader.h"
#include "CoreLib/Timer.h"


struct SAnimationCelIndex;
typedef void (*CelFunc)(void* psCel, float fFractionalTime, void* pvData);


struct SAnimationCelIndex
{
	void*		pvCel;		//What this cel is.  eg: a Position interpolator or a texture.
	float		fTime;	 	//How long to persist this frame.
	int			iNextCel;	//Index into animation.maCelIndices.
							//An iNextCel of -1 will not change to a new cel.
	CelFunc		Func;		//Function to call when this cel is played.
	void*		pvData;		//Arbitrary data which can be attached to this frame.
	int			iFlags;		//User flags.
};


enum EAnimationHappenOnceFlags
{
	AHOF_FirstTick = 1,
	AHOF_Occured = 2,
};


struct SAnimationHappenOnceData
{
	float		fFractionalTimeToOccur;
	CelFunc		Func;
};


class CAnimation;
typedef CArrayTemplate<SAnimationCelIndex>	CAnimationIndexCelArray;


class CAnimation : public CStandardTrackerObject
{
BASE_FUNCTIONS(CAnimation);
protected:
	CAnimationIndexCelArray		maCelIndices;
	BOOL						mbEnabled;
	int							miCurrentCel;
	float						mfCelTime;
	float						mfTimeMultiplier;

public:
	void 						Init(void);
	void 						Init(int iNumCels);
	void 						Kill(void);
	BOOL 						Save(CFileWriter* pcFile);
	BOOL						Load(CFileReader* pcFile);
	void 						Add(void* pvCel, float fTime, int iNextCel, CelFunc Func = NULL, void* pvData = NULL);
	void						Copy(CAnimation* pcSource);
	void* 						Update(void);
	void						Restart(void);
	SAnimationCelIndex* 		GetCurrentCel(void);
	float						GetFractionalCelTime(void);
	void*						GetCurrentCelPtr(void);
	void						Enable(void);
	void						Disable(void);
	void						SetSpeed(float fMultiplier);
	CAnimationIndexCelArray*	GetCelIndices(void);

	//Helper Functions.
	void						PointLastCelToFirst(void);
	void						MakeLoopingAnimation(void);
	void						MakeLoopOnceAnimation(void);
};


//This is an example of an animation cel function and is useful for changing textures.
void AnimationHappenOnceFunction(void* psCel, float fFractionalTime, void* pvData);


#endif //__ANIMATION_H__

