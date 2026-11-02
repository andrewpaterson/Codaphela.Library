#ifndef __SOUND_MIXER_H__
#define __SOUND_MIXER_H__
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
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "SoundVoice.h"


#define SOUND_MIXER_DEFAULT_SAMPLE_RATE		48000
#define SOUND_MIXER_DEFAULT_LEAD_MS			100


class CNativeSoundMixer;
class CNativeSoundFactory;


//Mixes its voices and plays them on the host, as a canvas draws its draws and shows them.  A window with a sound
//factory has one and mixes on each of its ticks.  Each Mix keeps the host muiLeadMs ahead of what it is playing: it
//mixes the frames the host has played since the last, adding up the voices at the mixer's sample rate, and takes out
//the voices that have finished.
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

public:
	void					Init(CNativeSoundFactory* pcSoundFactory, uint32 uiSampleRate);
	void					Class(void) override;
	void					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					AddVoice(Ptr<CSoundVoice> pVoice);
	bool					RemoveVoice(Ptr<CSoundVoice> pVoice);
	void					RemoveAllVoices(void);
	size					NumVoices(void);
	Ptr<CSoundVoice>		GetVoice(size uiIndex);

	void					Mix(void);
	void					Mix(lsize uiFrames);

	bool					SetSampleRate(uint32 uiSampleRate);
	uint32					GetSampleRate(void);
	void					SetLead(uint32 uiLeadMs);
	uint32					GetLead(void);
	void					SetVolume(float64 fVolume);
	float64					GetVolume(void);

	bool					IsOpen(void);
	uint64					NumFramesMixed(void);
	uint64					NumDroppedFrames(void);
	CNativeSoundMixer*		GetNativeMixer(void);

protected:
	void					RemoveFinishedVoices(void);
	void					SetMixFrames(lsize uiFrames);
};


#endif // __SOUND_MIXER_H__

