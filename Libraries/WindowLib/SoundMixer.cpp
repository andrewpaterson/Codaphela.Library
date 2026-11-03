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
#include "BaseLib/Logger.h"
#include "StandardLib/ClassDefines.h"
#include "NativeSoundFactory.h"
#include "SoundMixer.h"


//////////////////////////////////////////////////////////////////////////
//
// The host's output is opened at uiSampleRate.  If it can't be the mixer
// still takes voices but plays nothing.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Init(CNativeSoundFactory* pcSoundFactory, uint32 uiSampleRate)
{
	PreInit();

	maVoices.Init();
	muiSampleRate = uiSampleRate;
	muiLeadMs = SOUND_MIXER_DEFAULT_LEAD_MS;
	mfVolume = 1.0;
	mpfMix = NULL;
	mpaiFrames = NULL;
	muiMixFrames = 0;
	muiFramesMixed = 0;

	mpcNativeMixer = pcSoundFactory->CreateNativeSoundMixer(this);
	if (!mpcNativeMixer->Open(uiSampleRate))
	{
		gcLogger.Error2(__METHOD__, " Could not open the host's sound output: there is no sound.", NULL);
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Class(void)
{
	U_Pointer(mpcNativeMixer);
	M_Embedded(maVoices);
	U_UInt32(muiSampleRate);
	U_UInt32(muiLeadMs);
	U_Float64(mfVolume);
	U_Pointer(mpfMix);
	U_Pointer(mpaiFrames);
	U_LSize(muiMixFrames);
	U_UInt64(muiFramesMixed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Free(void)
{
	CNativeSoundFactory*	pcSoundFactory;

	if (mpcNativeMixer)
	{
		mpcNativeMixer->Close();
		pcSoundFactory = mpcNativeMixer->GetFactory();
		pcSoundFactory->DestroyNativeSoundMixer(mpcNativeMixer);
		mpcNativeMixer = NULL;
	}

	free(mpfMix);
	mpfMix = NULL;
	free(mpaiFrames);
	mpaiFrames = NULL;
	muiMixFrames = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::AddVoice(Ptr<CSoundVoice> pVoice)
{
	if (pVoice.IsNotNull())
	{
		maVoices.Add(pVoice);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::RemoveVoice(Ptr<CSoundVoice> pVoice)
{
	size	i;

	for (i = 0; i < maVoices.Size(); i++)
	{
		if (maVoices.Get(i) == pVoice)
		{
			maVoices.RemoveAt(i);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::RemoveAllVoices(void)
{
	while (maVoices.Size() > 0)
	{
		maVoices.RemoveAt(maVoices.Size() - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Mixes what the host needs to stay muiLeadMs ahead.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Mix(void)
{
	lsize	uiLeadFrames;
	lsize	uiQueuedFrames;

	if ((mpcNativeMixer == NULL) || !mpcNativeMixer->IsOpen())
	{
		return;
	}

	uiLeadFrames = ((lsize)muiSampleRate * muiLeadMs) / 1000;
	uiQueuedFrames = mpcNativeMixer->NumQueuedFrames();
	if (uiQueuedFrames < uiLeadFrames)
	{
		Mix(uiLeadFrames - uiQueuedFrames);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// While voices are playing the mixer must be mixed again before the host
// runs out of what it was given, so a window that waits for events wakes
// after this long.  A quarter of the lead leaves room for slow ticks.
// Returns false if nothing is playing.
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::GetMixDelay(uint32* puiMilliseconds)
{
	if ((mpcNativeMixer == NULL) || !mpcNativeMixer->IsOpen() || (maVoices.Size() == 0))
	{
		return false;
	}
	*puiMilliseconds = muiLeadMs / 4;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Mixes uiFrames and writes them to the host.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Mix(lsize uiFrames)
{
	size		i;
	lsize		uiFrame;
	float64*	pfLeft;
	float64*	pfRight;
	float64		fVolume;

	if ((uiFrames == 0) || (mpcNativeMixer == NULL) || !mpcNativeMixer->IsOpen())
	{
		return;
	}

	SetMixFrames(uiFrames);
	pfLeft = mpfMix;
	pfRight = &mpfMix[uiFrames];
	memset(mpfMix, 0, (size)(uiFrames * 2 * sizeof(float64)));

	for (i = 0; i < maVoices.Size(); i++)
	{
		maVoices.Get(i)->Play(pfLeft, pfRight, uiFrames, muiSampleRate);
	}
	RemoveFinishedVoices();

	fVolume = mfVolume;
	for (uiFrame = 0; uiFrame < uiFrames; uiFrame++)
	{
		ConvertSampleFromFloat(&mpaiFrames[uiFrame * 2], PT_int16, pfLeft[uiFrame] * fVolume);
		ConvertSampleFromFloat(&mpaiFrames[uiFrame * 2 + 1], PT_int16, pfRight[uiFrame] * fVolume);
	}

	mpcNativeMixer->Write(mpaiFrames, uiFrames);
	muiFramesMixed += uiFrames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::RemoveFinishedVoices(void)
{
	size	i;

	i = maVoices.Size();
	while (i > 0)
	{
		i--;
		if (maVoices.Get(i)->IsFinished())
		{
			maVoices.RemoveAt(i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::SetMixFrames(lsize uiFrames)
{
	if (uiFrames > muiMixFrames)
	{
		free(mpfMix);
		free(mpaiFrames);
		mpfMix = (float64*)malloc((size)(uiFrames * 2 * sizeof(float64)));
		mpaiFrames = (int16*)malloc((size)(uiFrames * 2 * sizeof(int16)));
		muiMixFrames = uiFrames;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the host's output again at uiSampleRate.
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::SetSampleRate(uint32 uiSampleRate)
{
	if (mpcNativeMixer == NULL)
	{
		return false;
	}

	mpcNativeMixer->Close();
	muiSampleRate = uiSampleRate;
	if (!mpcNativeMixer->Open(uiSampleRate))
	{
		gcLogger.Error2(__METHOD__, " Could not open the host's sound output: there is no sound.", NULL);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CSoundMixer::NumDroppedFrames(void)
{
	if (mpcNativeMixer == NULL)
	{
		return 0;
	}
	return mpcNativeMixer->NumDroppedFrames();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::IsOpen(void)
{
	return (mpcNativeMixer != NULL) && mpcNativeMixer->IsOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSoundMixer::NumVoices(void) { return maVoices.Size(); }
Ptr<CSoundVoice> CSoundMixer::GetVoice(size uiIndex) { return maVoices.Get(uiIndex); }
uint32 CSoundMixer::GetSampleRate(void) { return muiSampleRate; }
void CSoundMixer::SetLead(uint32 uiLeadMs) { muiLeadMs = uiLeadMs; }
uint32 CSoundMixer::GetLead(void) { return muiLeadMs; }
void CSoundMixer::SetVolume(float64 fVolume) { mfVolume = fVolume; }
float64 CSoundMixer::GetVolume(void) { return mfVolume; }
uint64 CSoundMixer::NumFramesMixed(void) { return muiFramesMixed; }
CNativeSoundMixer* CSoundMixer::GetNativeMixer(void) { return mpcNativeMixer; }

