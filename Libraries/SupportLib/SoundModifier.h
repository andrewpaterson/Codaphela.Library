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
#ifndef __SOUND_MODIFIER_H__
#define __SOUND_MODIFIER_H__
#include "StandardLib/Object.h"
#include "Sound.h"


//Sound Modifiers are applied to a sound by the Sound Modifier Stack.  Modify returns the sound it was given if it changes
//nothing, otherwise a new sound.
class CSoundModifier : public CObject
{
CONSTRUCTABLE(CSoundModifier);
DESTRUCTABLE(CSoundModifier);
public:
			void			Class(void) override;
			void			Free(void) override;

			bool			Save(CObjectWriter* pcFile) override;
			bool			Load(CObjectReader* pcFile) override;

	virtual Ptr<CSound>		Modify(Ptr<CSound> pcSound) =0;
};


#endif // __SOUND_MODIFIER_H__

