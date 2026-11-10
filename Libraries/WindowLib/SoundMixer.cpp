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
	mcMutex.Init();
	mcThread.Init();
	mbStop = false;
	mbRunning = false;

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
	U_Unknown(CMutex, mcMutex);
	U_Unknown(CNativeThread, mcThread);
	U_Bool(mbStop);
	U_Bool(mbRunning);
}


//////////////////////////////////////////////////////////////////////////
//
// The thread stops before anything it uses goes.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Free(void)
{
	CNativeSoundFactory*	pcSoundFactory;

	Stop();
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
	mcThread.Kill();
	mcMutex.Kill();
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
// The first lead is mixed straight away, and the thread keeps it up from
// then on.
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::Start(void)
{
	if (mbRunning)
	{
		return true;
	}

	Mix();
	Lock();
	mbStop = false;
	Unlock();
	mbRunning = mcThread.Start(ThreadFunction, this);
	return mbRunning;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns once the thread has stopped.  Nothing happens if it wasn't
// started.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Stop(void)
{
	if (!mbRunning)
	{
		return;
	}

	Lock();
	mbStop = true;
	Unlock();
	mcThread.Join();
	mbRunning = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::IsRunning(void)
{
	return mbRunning;
}


//////////////////////////////////////////////////////////////////////////
//
// Held while what a playing voice reads changes.  Not while making or
// letting go of objects the thread doesn't use.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Lock(void)
{
	mcMutex.Lock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Unlock(void)
{
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::AddVoice(Ptr<CSoundVoice> pVoice)
{
	if (pVoice.IsNotNull())
	{
		Lock();
		maVoices.Add(pVoice);
		Unlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSoundMixer::RemoveVoice(Ptr<CSoundVoice> pVoice)
{
	size	i;
	bool	bRemoved;

	bRemoved = false;
	Lock();
	for (i = 0; i < maVoices.NumElements(); i++)
	{
		if (maVoices.Get(i) == pVoice)
		{
			maVoices.RemoveAt(i);
			bRemoved = true;
			break;
		}
	}
	Unlock();
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::RemoveAllVoices(void)
{
	Lock();
	while (maVoices.NumElements() > 0)
	{
		maVoices.RemoveAt(maVoices.NumElements() - 1);
	}
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// On the window's thread: the voices that have finished go, and may be
// freed.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::RemoveFinishedVoices(void)
{
	size	i;

	Lock();
	i = maVoices.NumElements();
	while (i > 0)
	{
		i--;
		if (maVoices.Get(i)->IsFinished())
		{
			maVoices.RemoveAt(i);
		}
	}
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// Mixes what the host needs to stay muiLeadMs ahead, and takes out the
// voices that have finished.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Mix(void)
{
	Lock();
	MixAhead();
	Unlock();
	RemoveFinishedVoices();
}


//////////////////////////////////////////////////////////////////////////
//
// Mixes uiFrames, writes them to the host and takes out the voices that
// have finished.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Mix(lsize uiFrames)
{
	Lock();
	MixFrames(uiFrames);
	Unlock();
	RemoveFinishedVoices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::ThreadFunction(void* pvMixer)
{
	((CSoundMixer*)pvMixer)->Run();
}


//////////////////////////////////////////////////////////////////////////
//
// The thread: it keeps the host ahead until it's stopped.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::Run(void)
{
	uint32	uiSleepMs;
	bool	bStop;

	for (;;)
	{
		Lock();
		bStop = mbStop;
		if (!bStop)
		{
			MixAhead();
		}
		uiSleepMs = muiLeadMs / 4;
		Unlock();

		if (bStop)
		{
			return;
		}
		ThreadSleep((uiSleepMs > 0) ? (int)uiSleepMs : 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Under the lock.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::MixAhead(void)
{
	lsize	uiLeadFrames;
	lsize	uiQueuedFrames;

	if (!IsOpen())
	{
		return;
	}

	uiLeadFrames = ((lsize)muiSampleRate * muiLeadMs) / 1000;
	uiQueuedFrames = mpcNativeMixer->NumQueuedFrames();
	if (uiQueuedFrames < uiLeadFrames)
	{
		MixFrames(uiLeadFrames - uiQueuedFrames);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Under the lock, on either thread, so the voices are played through raw
// pointers: a Ptr made here would be made on the mixer's thread.
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::MixFrames(lsize uiFrames)
{
	size			i;
	lsize			uiFrame;
	float64*		pfLeft;
	float64*		pfRight;
	float64			fVolume;
	CSoundVoice*	pcVoice;

	if ((uiFrames == 0) || !IsOpen())
	{
		return;
	}

	SetMixFrames(uiFrames);
	pfLeft = mpfMix;
	pfRight = &mpfMix[uiFrames];
	memset(mpfMix, 0, (size)(uiFrames * 2 * sizeof(float64)));

	for (i = 0; i < maVoices.NumElements(); i++)
	{
		pcVoice = (CSoundVoice*)maVoices.UnsafeGetObject(i);
		if (!pcVoice->IsFinished())
		{
			pcVoice->Play(pfLeft, pfRight, uiFrames, muiSampleRate);
		}
	}

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
	bool	bResult;

	if (mpcNativeMixer == NULL)
	{
		return false;
	}

	Lock();
	mpcNativeMixer->Close();
	muiSampleRate = uiSampleRate;
	bResult = mpcNativeMixer->Open(uiSampleRate);
	Unlock();
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not open the host's sound output: there is no sound.", NULL);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CSoundMixer::NumDroppedFrames(void)
{
	uint64	uiDropped;

	if (mpcNativeMixer == NULL)
	{
		return 0;
	}
	Lock();
	uiDropped = mpcNativeMixer->NumDroppedFrames();
	Unlock();
	return uiDropped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CSoundMixer::NumFramesMixed(void)
{
	uint64	uiFramesMixed;

	Lock();
	uiFramesMixed = muiFramesMixed;
	Unlock();
	return uiFramesMixed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::SetLead(uint32 uiLeadMs)
{
	Lock();
	muiLeadMs = uiLeadMs;
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundMixer::SetVolume(float64 fVolume)
{
	Lock();
	mfVolume = fVolume;
	Unlock();
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
// The voices only change on the window's thread, so it can read them
// without the lock.
//
//////////////////////////////////////////////////////////////////////////
size CSoundMixer::NumVoices(void) { return maVoices.NumElements(); }
Ptr<CSoundVoice> CSoundMixer::GetVoice(size uiIndex) { return maVoices.Get(uiIndex); }
uint32 CSoundMixer::GetSampleRate(void) { return muiSampleRate; }
float64 CSoundMixer::GetVolume(void) { return mfVolume; }
CNativeSoundMixer* CSoundMixer::GetNativeMixer(void) { return mpcNativeMixer; }

