/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela PulseAudioLib

Codaphela PulseAudioLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela PulseAudioLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela PulseAudioLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <dlfcn.h>
#include "BaseLib/Logger.h"
#include "PulseAudioSoundMixer.h"


#define PULSE_STREAM_PLAYBACK		1			//PA_STREAM_PLAYBACK.
#define PULSE_SAMPLE_S16LE			3			//PA_SAMPLE_S16LE.
#define PULSE_DEFAULT				0xFFFFFFFF	//(uint32_t)-1: the server's choice.


struct SPulseSampleSpec
{
	int		iFormat;
	uint32	uiRate;
	uint8	uiChannels;
};


struct SPulseBufferAttr
{
	uint32	uiMaxLength;
	uint32	uiTargetLength;
	uint32	uiPreBuffer;
	uint32	uiMinimumRequest;
	uint32	uiFragmentSize;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPulseAudioSoundMixer::Init(CSoundMixer* pcMixer, CNativeSoundFactory* pcSoundFactory)
{
	CNativeSoundMixer::Init(pcMixer, pcSoundFactory);
	mpvLibrary = NULL;
	mpvSimple = NULL;
	mfWrite = NULL;
	mfGetLatency = NULL;
	mfFree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPulseAudioSoundMixer::Kill(void)
{
	Close();
	CNativeSoundMixer::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A playback stream of uiSampleRate 16 bit stereo frames.
//
//////////////////////////////////////////////////////////////////////////
bool CPulseAudioSoundMixer::Open(uint32 uiSampleRate)
{
	PulseSimpleNew		fNew;
	SPulseSampleSpec	sSpec;
	SPulseBufferAttr	sAttributes;
	int					iError;

	Close();
	if (uiSampleRate == 0)
	{
		return false;
	}

	mpvLibrary = dlopen("libpulse-simple.so.0", RTLD_NOW);
	if (mpvLibrary == NULL)
	{
		gcLogger.Error2(__METHOD__, " Could not load libpulse-simple.so.0.", NULL);
		return false;
	}
	fNew = (PulseSimpleNew)dlsym(mpvLibrary, "pa_simple_new");
	mfWrite = (PulseSimpleWrite)dlsym(mpvLibrary, "pa_simple_write");
	mfGetLatency = (PulseSimpleGetLatency)dlsym(mpvLibrary, "pa_simple_get_latency");
	mfFree = (PulseSimpleFree)dlsym(mpvLibrary, "pa_simple_free");
	if ((fNew == NULL) || (mfWrite == NULL) || (mfGetLatency == NULL) || (mfFree == NULL))
	{
		gcLogger.Error2(__METHOD__, " libpulse-simple.so.0 doesn't have the simple API.", NULL);
		Close();
		return false;
	}

	sSpec.iFormat = PULSE_SAMPLE_S16LE;
	sSpec.uiRate = uiSampleRate;
	sSpec.uiChannels = 2;
	sAttributes.uiMaxLength = PULSE_DEFAULT;
	sAttributes.uiTargetLength = (uiSampleRate * 4 * PULSE_AUDIO_MAX_QUEUED_MS) / 1000;
	sAttributes.uiPreBuffer = (uiSampleRate * 4 * PULSE_AUDIO_PRE_BUFFER_MS) / 1000;
	sAttributes.uiMinimumRequest = PULSE_DEFAULT;
	sAttributes.uiFragmentSize = PULSE_DEFAULT;
	iError = 0;
	mpvSimple = fNew(NULL, "Codaphela", PULSE_STREAM_PLAYBACK, NULL, "Sound Mixer", &sSpec, NULL, &sAttributes, &iError);
	if (mpvSimple == NULL)
	{
		gcLogger.Error2(__METHOD__, " Could not open a PulseAudio stream.", NULL);
		Close();
		return false;
	}

	muiSampleRate = uiSampleRate;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPulseAudioSoundMixer::Close(void)
{
	if (mpvSimple)
	{
		mfFree(mpvSimple);
		mpvSimple = NULL;
	}
	if (mpvLibrary)
	{
		dlclose(mpvLibrary);
		mpvLibrary = NULL;
	}
	mfWrite = NULL;
	mfGetLatency = NULL;
	mfFree = NULL;
	muiSampleRate = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CPulseAudioSoundMixer::NumQueuedFrames(void)
{
	uint64	uiMicroseconds;
	int		iError;

	if (mpvSimple == NULL)
	{
		return 0;
	}
	uiMicroseconds = mfGetLatency(mpvSimple, &iError);
	if (uiMicroseconds == (uint64)-1)
	{
		return 0;
	}
	return (lsize)((uiMicroseconds * muiSampleRate) / 1000000);
}


//////////////////////////////////////////////////////////////////////////
//
// pa_simple_write waits only when the server holds
// PULSE_AUDIO_MAX_QUEUED_MS: the mixer keeps it well under that.
//
//////////////////////////////////////////////////////////////////////////
void CPulseAudioSoundMixer::Write(int16* paiFrames, lsize uiFrames)
{
	int		iError;

	if (mpvSimple == NULL)
	{
		muiDroppedFrames += uiFrames;
		return;
	}
	if (mfWrite(mpvSimple, paiFrames, (size_t)uiFrames * 4, &iError) < 0)
	{
		muiDroppedFrames += uiFrames;
	}
}

