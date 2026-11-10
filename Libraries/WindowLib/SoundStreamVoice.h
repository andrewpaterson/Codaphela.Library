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
#ifndef __SOUND_STREAM_VOICE_H__
#define __SOUND_STREAM_VOICE_H__
#include "SoundVoice.h"


#define SOUND_STREAM_DEFAULT_LATENCY_MS			60
#define SOUND_STREAM_DEFAULT_MAX_LATENCY_MS		250


//Plays frames made as it goes, by an emulated sound chip for example.  Each Play the subclass's Fill is called until
//it has nothing more, each time filling the voice's buffer (a sound in the format given to Init) from its first
//frame.  The frames wait in the voice until they are played at the stream's sample rate.
//
//The stream and the host's output have clocks of their own, so the frames waiting go up and down.  Playing starts
//once the stream is muiLatencyMs ahead and, if it runs out, waits until it is that far ahead again.  If it gets
//more than muiMaxLatencyMs ahead the oldest frames are dropped to bring it back to muiLatencyMs.
class CSoundStreamVoice : public CSoundVoice
{
CONSTRUCTABLE(CSoundStreamVoice);
DESTRUCTABLE(CSoundStreamVoice);
protected:
	Ptr<CSound>		mpBuffer;
	float64*		mpfWaiting;			//Left and right pairs at muiStreamRate.
	lsize			muiWaitingFrames;
	lsize			muiWaitingCapacity;
	float64			mfPosition;			//Between the first two waiting frames.
	uint32			muiStreamRate;		//The rate of the waiting frames.
	bool			mbStarting;			//Waiting until the stream is far enough ahead.
	uint32			muiLatencyMs;
	uint32			muiMaxLatencyMs;
	float64*		mpfFill;			//One buffer of samples, left then right.
	uint64			muiFramesPlayed;	//Stream frames played.
	uint64			muiDroppedFrames;
	uint64			muiUnderruns;

public:
			void			Init(uint32 uiSampleRate, lsize uiBufferFrames, EPrimitiveType eType, bool bStereo);
			void			Class(void) override;
			void			Free(void) override;

			void			Play(float64* pfLeft, float64* pfRight, lsize uiFrames, uint32 uiSampleRate) override;

	virtual lsize			Fill(CSound* pcBuffer) =0;  //On the mixer's thread: see CSoundVoice.
	virtual uint32			GetStreamSampleRate(void);  //Zero when the stream isn't playing.

			void			SetLatency(uint32 uiLatencyMs, uint32 uiMaxLatencyMs);
			Ptr<CSound>		GetBuffer(void);
			lsize			NumWaitingFrames(void);
			uint64			NumFramesPlayed(void);
			uint64			NumDroppedFrames(void);
			uint64			NumUnderruns(void);

protected:
			void			Read(uint32 uiStreamRate);
			void			Append(lsize uiFrames);
			void			Discard(lsize uiFrames);
			void			Restart(void);
};


#endif // __SOUND_STREAM_VOICE_H__

