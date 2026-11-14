/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "StandardLib/ClassDefines.h"
#include "AnimationPlayer.h"


//////////////////////////////////////////////////////////////////////////
//
// From its start, as fast as it was made.
//
//////////////////////////////////////////////////////////////////////////
void CAnimationPlayer::Init(Ptr<CSceneAnimation> pAnimation, bool bLoop)
{
	Init(pAnimation, NULL, bLoop);
}


//////////////////////////////////////////////////////////////////////////
//
// A skeleton's animation, on pInstance.
//
//////////////////////////////////////////////////////////////////////////
void CAnimationPlayer::Init(Ptr<CSceneAnimation> pAnimation, Ptr<CInstance> pInstance, bool bLoop)
{
	size	i;
	bool	bSameSkeleton;

	PreInit();

	mpAnimation = pAnimation;
	mpInstance = pInstance;
	mapBound.Init();
	mfTime = 0.0f;
	mfSpeed = 1.0f;
	mbLoop = bLoop;

	if (pAnimation.IsNotNull() && pAnimation->IsSkeletal() && pInstance.IsNotNull())
	{
		bSameSkeleton = &pInstance->mpSkeleton == &pAnimation->mpSkeleton;
		for (i = 0; i < pAnimation->mapTracks.NumElements(); i++)
		{
			mapBound.Add(bSameSkeleton ? pInstance->GetBoneConnection(pAnimation->mapTracks.Get(i)->mpBone) : Ptr<CConnection>());
		}
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationPlayer::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationPlayer::Class(void)
{
	M_Pointer(mpAnimation);
	M_Pointer(mpInstance);
	M_Embedded(mapBound);
	U_Float32(mfTime);
	U_Float32(mfSpeed);
	U_Bool(mbLoop);
}


//////////////////////////////////////////////////////////////////////////
//
// Backwards too, with a negative speed.
//
//////////////////////////////////////////////////////////////////////////
void CAnimationPlayer::Advance(float32 fSeconds)
{
	SetTime(mfTime + fSeconds * mfSpeed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimationPlayer::Apply(void)
{
	size	i;

	if (mpAnimation.IsNull())
	{
		return;
	}

	if (mpAnimation->IsSkeletal())
	{
		for (i = 0; i < mapBound.NumElements(); i++)
		{
			mpAnimation->mapTracks.Get(i)->ApplyTo(mapBound.Get(i), mfTime);
		}
	}
	else
	{
		mpAnimation->Apply(mfTime);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Into the animation: round its length if it loops, and held at its start
// or end if it doesn't.
//
//////////////////////////////////////////////////////////////////////////
void CAnimationPlayer::SetTime(float32 fTime)
{
	float32	fDuration;

	fDuration = mpAnimation.IsNotNull() ? mpAnimation->GetDuration() : 0.0f;
	if (fDuration <= 0.0f)
	{
		mfTime = 0.0f;
	}
	else if (mbLoop)
	{
		mfTime = fmodf(fTime, fDuration);
		if (mfTime < 0.0f)
		{
			mfTime += fDuration;
		}
	}
	else
	{
		mfTime = (fTime < 0.0f) ? 0.0f : ((fTime > fDuration) ? fDuration : fTime);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One that doesn't loop, at its end.
//
//////////////////////////////////////////////////////////////////////////
bool CAnimationPlayer::IsFinished(void)
{
	if (mbLoop || mpAnimation.IsNull())
	{
		return false;
	}
	return mfTime >= mpAnimation->GetDuration();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CAnimationPlayer::GetTime(void) { return mfTime; }
void CAnimationPlayer::SetSpeed(float32 fSpeed) { mfSpeed = fSpeed; }
float32 CAnimationPlayer::GetSpeed(void) { return mfSpeed; }
Ptr<CSceneAnimation> CAnimationPlayer::GetAnimation(void) { return mpAnimation; }
Ptr<CInstance> CAnimationPlayer::GetInstance(void) { return mpInstance; }


//////////////////////////////////////////////////////////////////////////
//
// How many of a skeleton's animation's tracks found a connection.
//
//////////////////////////////////////////////////////////////////////////
size CAnimationPlayer::NumBound(void)
{
	size	i;
	size	uiBound;

	uiBound = 0;
	for (i = 0; i < mapBound.NumElements(); i++)
	{
		if (mapBound.Get(i).IsNotNull())
		{
			uiBound++;
		}
	}
	return uiBound;
}
