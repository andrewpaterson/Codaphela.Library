/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SoundCelVoice.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelVoice::Init(Ptr<CSoundCel> pCel)
{
	PreInit();
	PrivateInit(pCel);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelVoice::Init(Ptr<CSound> pSound, lsize uiLoopFrame)
{
	PreInit();
	PrivateInit(OMalloc<CSoundCel>(pSound, (lsize)0, pSound->GetNumFrames(), uiLoopFrame));
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelVoice::PrivateInit(Ptr<CSoundCel> pCel)
{
	CSoundVoice::Init();
	mpCel = pCel;
	mfPosition = 0.0;
	mpfWork = NULL;
	muiWorkFrames = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelVoice::Class(void)
{
	CSoundVoice::Class();
	M_Pointer(mpCel);
	U_Float64(mfPosition);
	U_Pointer(mpfWork);
	U_LSize(muiWorkFrames);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelVoice::Free(void)
{
	free(mpfWork);
	mpfWork = NULL;
	muiWorkFrames = 0;
	CSoundVoice::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// The cel's frames from the one before the first output frame to the one
// after the last are read (silence past the end of a cel without a
// loop) and interpolated at the cel's rate over the mixer's.
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelVoice::Play(float64* pfLeft, float64* pfRight, lsize uiFrames, uint32 uiSampleRate)
{
	Ptr<CSound>	pSound;
	size		uiLeft;
	size		uiRight;
	lsize		uiFirst;
	float64		fStart;
	float64		fStep;
	lsize		uiSpan;
	float64*	pfWorkLeft;
	float64*	pfWorkRight;
	lsize		i;
	float64		fPosition;

	if (mbFinished || (uiFrames == 0) || (uiSampleRate == 0) || mpCel.IsNull() || !mpCel->IsValid())
	{
		return;
	}

	pSound = mpCel->GetSourceSound();
	fStep = (float64)pSound->GetSampleRate() / (float64)uiSampleRate;
	uiFirst = (lsize)mfPosition;
	fStart = mfPosition - (float64)uiFirst;
	uiSpan = (lsize)(fStart + (float64)(uiFrames - 1) * fStep) + 2;

	GetStereoChannelIndices(&pSound, &uiLeft, &uiRight);
	pfWorkLeft = GetWork(uiSpan);
	pfWorkRight = &pfWorkLeft[uiSpan];
	mpCel->Play(uiLeft, uiFirst, uiSpan, pfWorkLeft);
	if (uiRight != uiLeft)
	{
		mpCel->Play(uiRight, uiFirst, uiSpan, pfWorkRight);
	}
	else
	{
		memcpy(pfWorkRight, pfWorkLeft, (size)(uiSpan * sizeof(float64)));
	}

	for (i = 0; i < uiFrames; i++)
	{
		fPosition = fStart + (float64)i * fStep;
		pfLeft[i] += Interpolate(pfWorkLeft, fPosition) * mfVolume;
		pfRight[i] += Interpolate(pfWorkRight, fPosition) * mfVolume;
	}

	mfPosition += (float64)uiFrames * fStep;
	if (!CSoundCel::WrapPosition(&mfPosition, mpCel->GetNumFrames(), mpCel->GetLoopFrame()))
	{
		mbFinished = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Room for uiFrames left samples followed by uiFrames right samples.
//
//////////////////////////////////////////////////////////////////////////
float64* CSoundCelVoice::GetWork(lsize uiFrames)
{
	if (uiFrames > muiWorkFrames)
	{
		free(mpfWork);
		mpfWork = (float64*)malloc((size)(uiFrames * 2 * sizeof(float64)));
		muiWorkFrames = uiFrames;
	}
	return mpfWork;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSoundCel> CSoundCelVoice::GetCel(void) { return mpCel; }
float64 CSoundCelVoice::GetPosition(void) { return mfPosition; }
void CSoundCelVoice::SetPosition(float64 fPosition) { mfPosition = fPosition; }

