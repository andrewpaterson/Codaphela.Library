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
#ifndef __SOUND_CEL_VOICE_H__
#define __SOUND_CEL_VOICE_H__
#include "SupportLib/SoundCel.h"
#include "SoundVoice.h"


//Plays a sound cel from its start, at its sound's sample rate, going round its loop if it has one.  A cel without a
//loop finishes at its end.  Samples between the cel's frames are interpolated linearly.
class CSoundCelVoice : public CSoundVoice
{
CONSTRUCTABLE(CSoundCelVoice);
DESTRUCTABLE(CSoundCelVoice);
protected:
	Ptr<CSoundCel>	mpCel;
	float64			mfPosition;		//Frames from the start of the cel.
	float64*		mpfWork;		//The cel's frames for one Play, left then right.
	lsize			muiWorkFrames;

public:
	void			Init(Ptr<CSoundCel> pCel);
	void			Init(Ptr<CSound> pSound, lsize uiLoopFrame);  //A cel of the whole sound; SOUND_CEL_NO_LOOP to play it once.
	void			Class(void) override;
	void			Free(void) override;

	void			Play(float64* pfLeft, float64* pfRight, lsize uiFrames, uint32 uiSampleRate) override;

	Ptr<CSoundCel>	GetCel(void);
	float64			GetPosition(void);
	void			SetPosition(float64 fPosition);

protected:
	void			PrivateInit(Ptr<CSoundCel> pCel);
	float64*		GetWork(lsize uiFrames);
};


#endif // __SOUND_CEL_VOICE_H__

