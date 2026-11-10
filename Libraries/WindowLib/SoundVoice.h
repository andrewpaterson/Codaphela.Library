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
#ifndef __SOUND_VOICE_H__
#define __SOUND_VOICE_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/Sound.h"


//One of the sounds a sound mixer plays, as a canvas draw is what a canvas shows.  Play adds the voice's next frames,
//at the mixer's sample rate, to what has been mixed so far.  A voice that has finished is taken out of the mixer.
//
//Play is called on the mixer's thread, under its lock (see CSoundMixer), so it mustn't make, copy or let go of a
//Ptr, nor make or free objects: the object graph isn't safe to use from two threads.  It reads the voice's objects
//through their Ptrs' -> and raw pointers.  SetVolume and Stop write one value each, which the thread reads.
class CSoundVoice : public CObject
{
CONSTRUCTABLE(CSoundVoice);
DESTRUCTABLE(CSoundVoice);
protected:
	float64			mfVolume;
	bool			mbFinished;

public:
			void	Init(void);
			void	Class(void) override;
			void	Free(void) override;

			bool	Save(CObjectWriter* pcFile) override;
			bool	Load(CObjectReader* pcFile) override;

	virtual void	Play(float64* pfLeft, float64* pfRight, lsize uiFrames, uint32 uiSampleRate) =0;

			void	SetVolume(float64 fVolume);
			float64	GetVolume(void);
			void	Stop(void);
			bool	IsFinished(void);

	static	void	GetStereoChannelIndices(CSound* pcSound, size* puiLeft, size* puiRight);
	static	float64	Interpolate(float64* pfSamples, float64 fPosition);
};


#endif // __SOUND_VOICE_H__

