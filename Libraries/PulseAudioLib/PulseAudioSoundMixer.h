#ifndef __PULSE_AUDIO_SOUND_MIXER_H__
#define __PULSE_AUDIO_SOUND_MIXER_H__
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
#include "WindowLib/NativeSoundMixer.h"


#define PULSE_AUDIO_MAX_QUEUED_MS		500		//How much the server will hold.
#define PULSE_AUDIO_PRE_BUFFER_MS		20		//How much it waits for before it starts, or starts again if it runs dry.


typedef void*	(*PulseSimpleNew)(const char* szServer, const char* szName, int iDirection, const char* szDevice, const char* szStreamName, const void* psSpec, const void* pvChannelMap, const void* psAttributes, int* piError);
typedef int		(*PulseSimpleWrite)(void* pvSimple, const void* pvData, size_t uiBytes, int* piError);
typedef uint64	(*PulseSimpleGetLatency)(void* pvSimple, int* piError);
typedef void	(*PulseSimpleFree)(void* pvSimple);


//A PulseAudio playback stream through its simple API (pulse/simple.h).  The API is declared here and its library,
//libpulse-simple.so.0, loaded when the stream is opened, so only the library is needed, not its development package.
//The frames queued are the stream's latency.
class CPulseAudioSoundMixer : public CNativeSoundMixer
{
CONSTRUCTABLE(CPulseAudioSoundMixer);
protected:
	void*					mpvLibrary;
	void*					mpvSimple;
	PulseSimpleWrite		mfWrite;
	PulseSimpleGetLatency	mfGetLatency;
	PulseSimpleFree			mfFree;

public:
	void	Init(CSoundMixer* pcMixer, CNativeSoundFactory* pcSoundFactory);
	void	Kill(void) override;

	bool	Open(uint32 uiSampleRate) override;
	void	Close(void) override;
	lsize	NumQueuedFrames(void) override;
	void	Write(int16* paiFrames, lsize uiFrames) override;
};


#endif // __PULSE_AUDIO_SOUND_MIXER_H__

