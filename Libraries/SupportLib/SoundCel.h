/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SOUND_CEL_H__
#define __SOUND_CEL_H__
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "Sound.h"


#define SOUND_CEL_NO_LOOP		MAX_LSIZE


//A run of frames in a source sound, so many logical sounds can share one sound (like an image cel in an image).  This is
//an entry in the SNES sample directory: where the sample starts and, if it loops, where it goes back to when it reaches
//its end.  The loop frame is counted from the cel's first frame.
class CSoundCel : public CObject
{
CONSTRUCTABLE(CSoundCel);
DESTRUCTABLE(CSoundCel);
protected:
	Ptr<CSound>		mpSourceSound;
	lsize			muiFirstFrame;
	lsize			muiNumFrames;
	lsize			muiLoopFrame;		//SOUND_CEL_NO_LOOP if the cel plays once.

public:
	void			Init(Ptr<CSound> pcSourceSound);
	void			Init(Ptr<CSound> pcSourceSound, lsize uiFirstFrame, lsize uiNumFrames, lsize uiLoopFrame = SOUND_CEL_NO_LOOP);
	void			Class(void) override;
	void			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			Copy(CSoundCel* pcSource);
	bool			IsValid(void);

	Ptr<CSound>		GetSourceSound(void);
	lsize			GetFirstFrame(void);
	lsize			GetNumFrames(void);
	bool			HasLoop(void);
	lsize			GetLoopFrame(void);
	lsize			GetLoopNumFrames(void);
	void			SetLoopFrame(lsize uiLoopFrame);

	float64			GetSample(lsize uiFrame, size uiChannelIndex);
	bool			GetSamples(size uiChannelIndex, lsize uiFirstFrame, lsize uiNumFrames, float64* pfDest);
	lsize			Play(size uiChannelIndex, lsize uiPosition, lsize uiNumFrames, float64* pfDest);
	Ptr<CSound>		ToSound(void);

	static lsize		GetLoopedFrame(lsize uiFrame, lsize uiNumFrames, lsize uiLoopFrame);
	static bool		WrapPosition(float64* pfPosition, lsize uiNumFrames, lsize uiLoopFrame);

	void			Print(CChars* psz);
	void			Dump(void);
};


typedef CArray<CSoundCel> CArraySoundCel;


#endif // __SOUND_CEL_H__

