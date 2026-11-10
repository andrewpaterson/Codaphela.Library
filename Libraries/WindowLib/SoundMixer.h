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
#ifndef __SOUND_MIXER_H__
#define __SOUND_MIXER_H__
#include "BaseLib/Mutex.h"
#include "BaseLib/NativeThread.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "SoundVoice.h"


#define SOUND_MIXER_DEFAULT_SAMPLE_RATE		48000
#define SOUND_MIXER_DEFAULT_LEAD_MS			100


class CNativeSoundMixer;
class CNativeSoundFactory;


//Mixes its voices and plays them on the host, as a canvas draws its draws and shows them.  Each Mix keeps the host
//muiLeadMs ahead of what it is playing: it mixes the frames the host has played since the last, adding up the voices at
//the mixer's sample rate, and takes out the voices that have finished.
//
//A window with a sound factory has one, which mixes on a thread of its own while the window is shown (Start and Stop),
//so that however long the window's thread is busy, or a dialog over it stops its ticks, what's playing is still heard.
//The thread wakes every quarter of the lead and mixes under the mixer's lock.  The object graph isn't safe to use from
//two threads, so the thread makes no Ptr: it plays the voices through raw pointers, and a voice's Play mustn't make,
//copy or let go of a Ptr either (see CSoundVoice).  Voices are only added, removed and let go on the window's thread,
//under the lock, so the thread never frees one: one that has finished waits for RemoveFinishedVoices, which the window
//calls each tick.  Anything that changes what a playing voice reads takes Lock first.
//
//Without Start nothing mixes but Mix, which is how the tests drive it.
class CSoundMixer : public CObject
{
CONSTRUCTABLE(CSoundMixer);
DESTRUCTABLE(CSoundMixer);
protected:
	CNativeSoundMixer*		mpcNativeMixer;
	CArray<CSoundVoice>		maVoices;
	uint32					muiSampleRate;
	uint32					muiLeadMs;
	float64					mfVolume;
	float64*				mpfMix;			//Left samples then right samples.
	int16*					mpaiFrames;
	lsize					muiMixFrames;
	uint64					muiFramesMixed;

	CMutex					mcMutex;		//Over all of the above, the voices and the host's output.
	CNativeThread			mcThread;
	bool					mbStop;			//Under mcMutex: the thread is to stop.
	bool					mbRunning;		//The window's thread's: Start started the thread.

public:
	void					Init(CNativeSoundFactory* pcSoundFactory, uint32 uiSampleRate);
	void					Class(void) override;
	void					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	bool					Start(void);
	void					Stop(void);
	bool					IsRunning(void);
	void					Lock(void);
	void					Unlock(void);

	void					AddVoice(Ptr<CSoundVoice> pVoice);
	bool					RemoveVoice(Ptr<CSoundVoice> pVoice);
	void					RemoveAllVoices(void);
	void					RemoveFinishedVoices(void);
	size					NumVoices(void);
	Ptr<CSoundVoice>		GetVoice(size uiIndex);

	void					Mix(void);
	void					Mix(lsize uiFrames);

	bool					SetSampleRate(uint32 uiSampleRate);
	uint32					GetSampleRate(void);
	void					SetLead(uint32 uiLeadMs);
	void					SetVolume(float64 fVolume);
	float64					GetVolume(void);

	bool					IsOpen(void);
	uint64					NumFramesMixed(void);
	uint64					NumDroppedFrames(void);
	CNativeSoundMixer*		GetNativeMixer(void);

protected:
	static void				ThreadFunction(void* pvMixer);
	void					Run(void);
	void					MixAhead(void);
	void					MixFrames(lsize uiFrames);
	void					SetMixFrames(lsize uiFrames);
};


#endif // __SOUND_MIXER_H__
