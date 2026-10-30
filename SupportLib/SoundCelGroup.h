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
#ifndef __SOUND_CEL_GROUP_H__
#define __SOUND_CEL_GROUP_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "SoundCel.h"


//A sound bank and its sample directory: one sound holding many sounds back to back and a cel for each, found by index
//(the SNES's source number).  AddSound copies a sound onto the end of the bank, converting its samples to the bank's
//sample types, and gives it the next index.
class CSoundCelGroup : public CObject
{
CONSTRUCTABLE(CSoundCelGroup);
DESTRUCTABLE(CSoundCelGroup);
protected:
	CArraySoundCel	maSoundCels;
	Ptr<CSound>		mpSound;

public:
	void 			Init(Ptr<CSound> pcSound);
	void 			Free(void) override;
	void			Class(void) override;

	bool			Load(CObjectReader* pcFile) override;
	bool			Save(CObjectWriter* pcFile) override;

	size			AddCel(Ptr<CSoundCel> pCel);
	size			AddCel(lsize uiFirstFrame, lsize uiNumFrames, lsize uiLoopFrame = SOUND_CEL_NO_LOOP);
	size			AddSound(Ptr<CSound> pcSound, lsize uiLoopFrame = SOUND_CEL_NO_LOOP);

	size			NumCels(void);
	Ptr<CSoundCel>	GetCel(size uiIndex);
	Ptr<CSound>		GetSound(void);
};


typedef CArray<CSoundCelGroup>	CArraySoundCelGroup;


#endif // __SOUND_CEL_GROUP_H__

