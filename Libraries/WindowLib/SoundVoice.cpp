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
#include "BaseLib/ArrayElementNotFound.h"
#include "StandardLib/ClassDefines.h"
#include "SoundVoice.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundVoice::Init(void)
{
	PreInit();
	mfVolume = 1.0;
	mbFinished = false;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundVoice::Class(void)
{
	U_Float64(mfVolume);
	U_Bool(mbFinished);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundVoice::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundVoice::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundVoice::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The channels a sound plays on the left and right speakers: its left
// and right channels, otherwise its mono channel, otherwise its first.
// A sound with only one of them plays it on both.
//
//////////////////////////////////////////////////////////////////////////
void CSoundVoice::GetStereoChannelIndices(CSound* pcSound, size* puiLeft, size* puiRight)
{
	size	uiLeft;
	size	uiRight;

	uiLeft = pcSound->GetIndexOfChannel(SOUND_LEFT);
	uiRight = pcSound->GetIndexOfChannel(SOUND_RIGHT);
	if (uiLeft == ARRAY_ELEMENT_NOT_FOUND)
	{
		uiLeft = uiRight;
	}
	if (uiLeft == ARRAY_ELEMENT_NOT_FOUND)
	{
		uiLeft = pcSound->GetIndexOfChannel(SOUND_MONO);
		if (uiLeft == ARRAY_ELEMENT_NOT_FOUND)
		{
			uiLeft = 0;
		}
	}
	if (uiRight == ARRAY_ELEMENT_NOT_FOUND)
	{
		uiRight = uiLeft;
	}
	*puiLeft = uiLeft;
	*puiRight = uiRight;
}


//////////////////////////////////////////////////////////////////////////
//
// Linear between the samples either side of fPosition; the sample after
// it must be there.
//
//////////////////////////////////////////////////////////////////////////
float64 CSoundVoice::Interpolate(float64* pfSamples, float64 fPosition)
{
	lsize	uiIndex;
	float64	fFraction;

	uiIndex = (lsize)fPosition;
	fFraction = fPosition - (float64)uiIndex;
	return pfSamples[uiIndex] + (pfSamples[uiIndex + 1] - pfSamples[uiIndex]) * fFraction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundVoice::SetVolume(float64 fVolume) { mfVolume = fVolume; }
float64 CSoundVoice::GetVolume(void) { return mfVolume; }
void CSoundVoice::Stop(void) { mbFinished = true; }
bool CSoundVoice::IsFinished(void) { return mbFinished; }

