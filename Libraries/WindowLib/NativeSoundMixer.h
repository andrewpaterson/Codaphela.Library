#ifndef __NATIVE_SOUND_MIXER_H__
#define __NATIVE_SOUND_MIXER_H__
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
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "BaseLib/PrimitiveTypes.h"


class CSoundMixer;
class CNativeSoundFactory;


//The host's sound output for a sound mixer: it plays the mixer's 16 bit signed stereo frames (left then right).  Write
//queues the frames and returns at once; frames that don't fit are dropped and counted.  NumQueuedFrames is how many
//of the frames written are still to be played, so the mixer knows how many more to mix.
class CNativeSoundMixer : public CConstructable, public CKillable
{
CONSTRUCTABLE(CNativeSoundMixer);
protected:
	CSoundMixer*			mpcMixer;
	CNativeSoundFactory*	mpcSoundFactory;
	uint32					muiSampleRate;		//Zero when it isn't open.
	uint64					muiDroppedFrames;

public:
			void					Init(CSoundMixer* pcMixer, CNativeSoundFactory* pcSoundFactory);
			void					Kill(void) override;

	virtual bool					Open(uint32 uiSampleRate) =0;
	virtual void					Close(void) =0;
	virtual lsize					NumQueuedFrames(void) =0;
	virtual void					Write(int16* paiFrames, lsize uiFrames) =0;

			bool					IsOpen(void);
			uint32					GetSampleRate(void);
			uint64					NumDroppedFrames(void);
			CSoundMixer*			GetMixer(void);
			CNativeSoundFactory*	GetFactory(void);
};


#endif // __NATIVE_SOUND_MIXER_H__

