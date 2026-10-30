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


#define SOUND_CEL_NO_LOOP		MAX_SIZE


//A run of frames in a source sound, so many logical sounds can share one sound (like an image cel in an image).  This is
//an entry in the SNES sample directory: where the sample starts and, if it loops, where it goes back to when it reaches
//its end.  The loop frame is counted from the cel's first frame.
class CSoundCel : public CObject
{
CONSTRUCTABLE(CSoundCel);
DESTRUCTABLE(CSoundCel);
protected:
	Ptr<CSound>		mpSourceSound;
	size			muiFirstFrame;
	size			muiNumFrames;
	size			muiLoopFrame;		//SOUND_CEL_NO_LOOP if the cel plays once.

public:
	void			Init(Ptr<CSound> pcSourceSound);
	void			Init(Ptr<CSound> pcSourceSound, size uiFirstFrame, size uiNumFrames, size uiLoopFrame = SOUND_CEL_NO_LOOP);
	void			Class(void) override;
	void			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			Copy(CSoundCel* pcSource);
	bool			IsValid(void);

	Ptr<CSound>		GetSourceSound(void);
	size			GetFirstFrame(void);
	size			GetNumFrames(void);
	bool			HasLoop(void);
	size			GetLoopFrame(void);
	size			GetLoopNumFrames(void);
	void			SetLoopFrame(size uiLoopFrame);

	float64			GetSample(size uiFrame, size uiChannelIndex);
	bool			GetSamples(size uiChannelIndex, size uiFirstFrame, size uiNumFrames, float64* pfDest);
	size			Play(size uiChannelIndex, size uiPosition, size uiNumFrames, float64* pfDest);
	Ptr<CSound>		ToSound(void);

	static size		GetLoopedFrame(size uiFrame, size uiNumFrames, size uiLoopFrame);
	static bool		WrapPosition(float64* pfPosition, size uiNumFrames, size uiLoopFrame);

	void			Print(CChars* psz);
	void			Dump(void);
};


typedef CArray<CSoundCel> CArraySoundCel;


#endif // __SOUND_CEL_H__

