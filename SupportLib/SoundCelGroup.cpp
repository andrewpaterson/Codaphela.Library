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

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "StandardLib/ClassDefines.h"
#include "SoundCelGroup.h"


//////////////////////////////////////////////////////////////////////////
//
// pcSound is the bank.  It can start with no frames.
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelGroup::Init(Ptr<CSound> pcSound)
{
	PreInit();

	mpSound = pcSound;
	maSoundCels.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelGroup::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundCelGroup::Class(void)
{
	M_Embedded(maSoundCels);
	M_Pointer(mpSound);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundCelGroup::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundCelGroup::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The cel must be in the bank.  Returns its index, or
// ARRAY_ELEMENT_NOT_FOUND.
//
//////////////////////////////////////////////////////////////////////////
size CSoundCelGroup::AddCel(Ptr<CSoundCel> pCel)
{
	if (pCel.IsNull() || (pCel->GetSourceSound() != mpSound) || !pCel->IsValid())
	{
		gcLogger.Error2(__METHOD__, " Cel is not in the group's sound.", NULL);
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	maSoundCels.Add(pCel);
	return maSoundCels.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSoundCelGroup::AddCel(size uiFirstFrame, size uiNumFrames, size uiLoopFrame)
{
	Ptr<CSoundCel>	pCel;

	pCel = OMalloc<CSoundCel>(mpSound, uiFirstFrame, uiNumFrames, uiLoopFrame);
	return AddCel(pCel);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies pcSound onto the end of the bank and adds a cel for it.  It
// must have the bank's sample rate and number of channels; channels are
// matched by index.  Returns the cel's index, or ARRAY_ELEMENT_NOT_FOUND.
//
//////////////////////////////////////////////////////////////////////////
size CSoundCelGroup::AddSound(Ptr<CSound> pcSound, size uiLoopFrame)
{
	size		uiFirstFrame;
	size		uiNumFrames;
	size		uiChannel;
	float64*	pfSamples;
	bool		bResult;

	if (pcSound.IsNull() || !pcSound->CanAccessSamples() || !mpSound->CanAccessSamples())
	{
		gcLogger.Error2(__METHOD__, " Sound samples cannot be read.", NULL);
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	if (pcSound->GetSampleRate() != mpSound->GetSampleRate())
	{
		gcLogger.Error2(__METHOD__, " Sound sample rate [", IntToString(pcSound->GetSampleRate()), "] is not the bank's [", IntToString(mpSound->GetSampleRate()), "].", NULL);
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	if (pcSound->NumChannels() != mpSound->NumChannels())
	{
		gcLogger.Error2(__METHOD__, " Sound has [", SizeToString(pcSound->NumChannels()), "] channels but the bank has [", SizeToString(mpSound->NumChannels()), "].", NULL);
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	uiFirstFrame = mpSound->GetNumFrames();
	uiNumFrames = pcSound->GetNumFrames();
	mpSound->BeginChange();
	mpSound->SetNumFrames(uiFirstFrame + uiNumFrames);
	if (!mpSound->EndChange() && (uiFirstFrame + uiNumFrames > 0))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	bResult = true;
	pfSamples = (float64*)malloc(sizeof(float64) * (uiNumFrames + 1));
	for (uiChannel = 0; uiChannel < pcSound->NumChannels(); uiChannel++)
	{
		bResult &= pcSound->GetSamples(uiChannel, 0, uiNumFrames, pfSamples);
		bResult &= mpSound->SetSamples(uiChannel, uiFirstFrame, uiNumFrames, pfSamples);
	}
	free(pfSamples);
	if (!bResult)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	return AddCel(uiFirstFrame, uiNumFrames, uiLoopFrame);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSoundCelGroup::NumCels(void)
{
	return maSoundCels.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSoundCel> CSoundCelGroup::GetCel(size uiIndex)
{
	return maSoundCels.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> CSoundCelGroup::GetSound(void)
{
	return mpSound;
}

