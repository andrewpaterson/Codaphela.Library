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
#include "Animation.h"
#include "BaseLib/IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::Init(void)
{
	mfTimeMultiplier = 1.0f;
	mfCelTime = 0.0f;
	maCelIndices.Init();
	mbEnabled = true;
	miCurrentCel = 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::Kill(void)
{
	maCelIndices.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAnimation::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteBool(mbEnabled));
	ReturnOnFalse(pcFile->WriteInt32(miCurrentCel));
	ReturnOnFalse(maCelIndices.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAnimation::Load(CFileReader* pcFile)
{
	Init();
	ReturnOnFalse(pcFile->ReadBool(&mbEnabled));
	ReturnOnFalse(pcFile->ReadInt32(&miCurrentCel));
	ReturnOnFalse(maCelIndices.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::Add(void* pvCel, float fTime, int iNextCel, CelFunc Func, void* pvData)
{
	SAnimationCelIndex*		psCel;

	psCel = maCelIndices.Add();

	psCel->fTime = fTime;
	psCel->pvCel = pvCel;
	psCel->iNextCel = iNextCel;
	psCel->Func = Func;
	psCel->pvData = pvData;
	psCel->iFlags = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CAnimation::Update(void)
{
	SAnimationCelIndex*		psCel;
	float					fFrac;
	bool					bNewFrame;

	psCel = maCelIndices.SafeGet(miCurrentCel);
	if (psCel)
	{
		mfCelTime += gcTimer.GetUpdateTimeInSeconds() * mfTimeMultiplier;
		bNewFrame = false;

		do 
		{
			if (mfCelTime >= psCel->fTime)
			{
				if (psCel->iNextCel != -1)
				{
					miCurrentCel = psCel->iNextCel;
					bNewFrame = true;
				}
				mfCelTime -= psCel->fTime;  //FIX TIMER.
			}
			else if (mfCelTime == 0.0f)
			{
				bNewFrame = true;
			}

			psCel = maCelIndices.SafeGet(miCurrentCel);
			if (psCel)
			{
				if (bNewFrame)
				{
					SetFlagInt(&psCel->iFlags, AHOF_FirstTick, true);
				}
				else
				{
					SetFlagInt(&psCel->iFlags, AHOF_FirstTick, false);
				}
				if (psCel->Func)
				{
					fFrac = GetFractionalCelTime();
					psCel->Func(psCel->pvCel, fFrac, psCel->pvData);
				}
				if (mfCelTime < psCel->fTime)
				{
					return psCel->pvCel;
				}
			}
			else
			{
				return NULL;
			}
		} while (mfCelTime >= psCel->fTime);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAnimationCelIndex* CAnimation::GetCurrentCel(void)
{
	return maCelIndices.SafeGet(miCurrentCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CAnimation::GetCurrentCelPtr(void)
{
	SAnimationCelIndex*		psCel;

	psCel = maCelIndices.SafeGet(miCurrentCel);
	if (psCel)
	{
		return psCel->pvCel;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CAnimation::GetFractionalCelTime(void)
{
	SAnimationCelIndex*		psCel;
	float					fTime;

	psCel = maCelIndices.SafeGet(miCurrentCel);
	if (psCel)
	{
		if (psCel->fTime != 0)
		{
			//Get time in the range ~ 0.0f to 1.f
			fTime = mfCelTime / psCel->fTime;

			//It is possible that fTime be outside the range because of slow framerates.
			//Clamp it.
			if (fTime < 0.0f)
			{
				return 0.0f;
			}
			else if (fTime > 1.0f)
			{
				return 1.0f;
			}
			return fTime;
		}
		return 0.0f;
	}
	return 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::Restart(void)
{
	mfCelTime = 0.0f;
	miCurrentCel = 0;
	mbEnabled = true;
	mfTimeMultiplier = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::Enable(void)
{
	mbEnabled = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::Disable(void)
{
	mbEnabled = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::SetSpeed(float fMultiplier)
{
	mfTimeMultiplier = fMultiplier;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimationIndexCelArray* CAnimation::GetCelIndices(void)
{
	return &maCelIndices;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::Copy(CAnimation* pcSource)
{
	Init();
	maCelIndices.Copy(&pcSource->maCelIndices);
	mbEnabled = pcSource->mbEnabled;
	miCurrentCel = pcSource->miCurrentCel;
	mfCelTime = pcSource->mfCelTime;
	mfTimeMultiplier = pcSource->mfTimeMultiplier;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::PointLastCelToFirst(void)
{
	SAnimationCelIndex*	psCel;

	psCel = maCelIndices.Get(maCelIndices.NumElements()-1);
	psCel->iNextCel = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::MakeLoopingAnimation(void)
{
	SAnimationCelIndex*	psCel;
	size				i;
	size				uiNumElements;

	uiNumElements = maCelIndices.NumElements();
	if (uiNumElements > 1)
	{
		for (i = 0; i < uiNumElements - 1; i++)
		{
			psCel = maCelIndices.Get(i);
			psCel->iNextCel = i + 1;
		}
		PointLastCelToFirst();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnimation::MakeLoopOnceAnimation(void)
{
	SAnimationCelIndex*	psCel;
	size				i;
	size				uiNumElements;

	uiNumElements = maCelIndices.NumElements();
	if (uiNumElements > 1)
	{
		psCel = NULL;
		for (i = 0; i < uiNumElements - 1; i++)
		{
			psCel = maCelIndices.Get(i);
			psCel->iNextCel = i + 1;
		}
		psCel->iNextCel = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AnimationHappenOnceFunction(void* pvCel, float fFractionalTime, void* pvData)
{
	//The data into the happen once function is a cel.
	//I think this may need looking at again.
	SAnimationHappenOnceData*	psHappenOnceData;
	SAnimationCelIndex*			psCel;

	psCel = (SAnimationCelIndex*)pvCel;

	psHappenOnceData = (SAnimationHappenOnceData*)pvData;
	if (psCel->iFlags & AHOF_FirstTick) 
	{
		SetFlagInt(&psCel->iFlags, AHOF_Occured, false);
	}
	
	if (psHappenOnceData->fFractionalTimeToOccur >= fFractionalTime)
	{
		SetFlagInt(&psCel->iFlags, AHOF_Occured, true);
		if (psHappenOnceData->Func)
		{
			psHappenOnceData->Func(psCel->pvCel, fFractionalTime, psCel->pvData);
		}
	}
}


