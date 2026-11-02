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
#include "SoundStreamVoice.h"


//////////////////////////////////////////////////////////////////////////
//
// The buffer Fill writes into: uiBufferFrames frames of eType, left and
// right or mono.
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::Init(uint32 uiSampleRate, lsize uiBufferFrames, EPrimitiveType eType, bool bStereo)
{
	PreInit();

	CSoundVoice::Init();
	if (bStereo)
	{
		mpBuffer = OMalloc<CSound>(uiSampleRate, uiBufferFrames, eType, SOUND_LEFT, SOUND_RIGHT, CHANNEL_STOP);
	}
	else
	{
		mpBuffer = OMalloc<CSound>(uiSampleRate, uiBufferFrames, eType, SOUND_MONO, CHANNEL_STOP);
	}
	mpfWaiting = NULL;
	muiWaitingFrames = 0;
	muiWaitingCapacity = 0;
	mfPosition = 0.0;
	muiStreamRate = 0;
	mbStarting = true;
	muiLatencyMs = SOUND_STREAM_DEFAULT_LATENCY_MS;
	muiMaxLatencyMs = SOUND_STREAM_DEFAULT_MAX_LATENCY_MS;
	mpfFill = (float64*)malloc((size)(uiBufferFrames * 2 * sizeof(float64)));
	muiFramesPlayed = 0;
	muiDroppedFrames = 0;
	muiUnderruns = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::Class(void)
{
	CSoundVoice::Class();
	M_Pointer(mpBuffer);
	U_Pointer(mpfWaiting);
	U_LSize(muiWaitingFrames);
	U_LSize(muiWaitingCapacity);
	U_Float64(mfPosition);
	U_UInt32(muiStreamRate);
	U_Bool(mbStarting);
	U_UInt32(muiLatencyMs);
	U_UInt32(muiMaxLatencyMs);
	U_Pointer(mpfFill);
	U_UInt64(muiFramesPlayed);
	U_UInt64(muiDroppedFrames);
	U_UInt64(muiUnderruns);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::Free(void)
{
	free(mpfWaiting);
	mpfWaiting = NULL;
	free(mpfFill);
	mpfFill = NULL;
	CSoundVoice::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// The stream's new frames are read, then the waiting frames played at
// the stream's rate over the mixer's.
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::Play(float64* pfLeft, float64* pfRight, lsize uiFrames, uint32 uiSampleRate)
{
	uint32		uiStreamRate;
	lsize		uiLatencyFrames;
	lsize		uiMaxFrames;
	float64		fStep;
	float64		fPosition;
	lsize		uiIndex;
	float64		fFraction;
	float64*	pf;
	lsize		i;
	lsize		uiConsumed;

	uiStreamRate = GetStreamSampleRate();
	if (uiStreamRate != muiStreamRate)
	{
		muiStreamRate = uiStreamRate;
		Restart();
	}
	Read(uiStreamRate);
	if (mbFinished || (uiStreamRate == 0) || (uiSampleRate == 0) || (uiFrames == 0))
	{
		return;
	}

	uiLatencyFrames = ((lsize)uiStreamRate * muiLatencyMs) / 1000;
	uiMaxFrames = ((lsize)uiStreamRate * muiMaxLatencyMs) / 1000;
	if (muiWaitingFrames > uiMaxFrames)
	{
		muiDroppedFrames += muiWaitingFrames - uiLatencyFrames;
		Discard(muiWaitingFrames - uiLatencyFrames);
	}

	if (mbStarting)
	{
		//Interpolating needs two frames.
		if ((muiWaitingFrames < uiLatencyFrames) || (muiWaitingFrames < 2))
		{
			return;
		}
		mbStarting = false;
	}

	fStep = (float64)uiStreamRate / (float64)uiSampleRate;
	for (i = 0; i < uiFrames; i++)
	{
		fPosition = mfPosition + (float64)i * fStep;
		uiIndex = (lsize)fPosition;
		if (uiIndex + 1 >= muiWaitingFrames)
		{
			//Run out: what is left waits for the stream to get ahead again.
			muiUnderruns++;
			mbStarting = true;
			break;
		}
		fFraction = fPosition - (float64)uiIndex;
		pf = &mpfWaiting[uiIndex * 2];
		pfLeft[i] += (pf[0] + (pf[2] - pf[0]) * fFraction) * mfVolume;
		pfRight[i] += (pf[1] + (pf[3] - pf[1]) * fFraction) * mfVolume;
	}

	fPosition = mfPosition + (float64)i * fStep;
	uiConsumed = (lsize)fPosition;
	mfPosition = fPosition - (float64)uiConsumed;
	Discard(uiConsumed);
	muiFramesPlayed += uiConsumed;
}


//////////////////////////////////////////////////////////////////////////
//
// Fills until the stream has no more.  Frames from a stream that isn't
// playing (no sample rate) are thrown away.
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::Read(uint32 uiStreamRate)
{
	lsize	uiBufferFrames;
	lsize	uiFilled;

	uiBufferFrames = mpBuffer->GetNumFrames();
	do
	{
		uiFilled = Fill(mpBuffer);
		if (uiFilled > uiBufferFrames)
		{
			uiFilled = uiBufferFrames;
		}
		if ((uiFilled > 0) && (uiStreamRate != 0))
		{
			Append(uiFilled);
		}
	}
	while ((uiFilled == uiBufferFrames) && (uiFilled > 0));
}


//////////////////////////////////////////////////////////////////////////
//
// The first uiFrames of the buffer onto the end of the waiting frames.
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::Append(lsize uiFrames)
{
	size		uiLeft;
	size		uiRight;
	lsize		uiBufferFrames;
	float64*	pfFillLeft;
	float64*	pfFillRight;
	float64*	pf;
	lsize		i;

	uiBufferFrames = mpBuffer->GetNumFrames();
	pfFillLeft = mpfFill;
	pfFillRight = &mpfFill[uiBufferFrames];
	GetStereoChannelIndices(&mpBuffer, &uiLeft, &uiRight);
	mpBuffer->GetSamples(uiLeft, 0, uiFrames, pfFillLeft);
	mpBuffer->GetSamples(uiRight, 0, uiFrames, pfFillRight);

	if (muiWaitingFrames + uiFrames > muiWaitingCapacity)
	{
		muiWaitingCapacity = (muiWaitingFrames + uiFrames) * 2;
		mpfWaiting = (float64*)realloc(mpfWaiting, (size)(muiWaitingCapacity * 2 * sizeof(float64)));
	}

	pf = &mpfWaiting[muiWaitingFrames * 2];
	for (i = 0; i < uiFrames; i++)
	{
		pf[i * 2] = pfFillLeft[i];
		pf[i * 2 + 1] = pfFillRight[i];
	}
	muiWaitingFrames += uiFrames;
}


//////////////////////////////////////////////////////////////////////////
//
// The oldest uiFrames waiting frames.
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::Discard(lsize uiFrames)
{
	if (uiFrames >= muiWaitingFrames)
	{
		muiWaitingFrames = 0;
		return;
	}
	if (uiFrames > 0)
	{
		muiWaitingFrames -= uiFrames;
		memmove(mpfWaiting, &mpfWaiting[uiFrames * 2], (size)(muiWaitingFrames * 2 * sizeof(float64)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The stream's rate changed (or it stopped): the frames waiting were at
// the old rate.
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::Restart(void)
{
	muiWaitingFrames = 0;
	mfPosition = 0.0;
	mbStarting = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CSoundStreamVoice::GetStreamSampleRate(void)
{
	return mpBuffer->GetSampleRate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSoundStreamVoice::SetLatency(uint32 uiLatencyMs, uint32 uiMaxLatencyMs)
{
	muiLatencyMs = uiLatencyMs;
	muiMaxLatencyMs = (uiMaxLatencyMs > uiLatencyMs) ? uiMaxLatencyMs : uiLatencyMs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSound> CSoundStreamVoice::GetBuffer(void) { return mpBuffer; }
lsize CSoundStreamVoice::NumWaitingFrames(void) { return muiWaitingFrames; }
uint64 CSoundStreamVoice::NumFramesPlayed(void) { return muiFramesPlayed; }
uint64 CSoundStreamVoice::NumDroppedFrames(void) { return muiDroppedFrames; }
uint64 CSoundStreamVoice::NumUnderruns(void) { return muiUnderruns; }

