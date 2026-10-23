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
#ifndef __SOUND_MODIFIER_STACK_H__
#define __SOUND_MODIFIER_STACK_H__
#include "StandardLib/Object.h"
#include "StandardLib/Objects.h"
#include "StandardLib/Array.h"
#include "Sound.h"
#include "SoundModifier.h"


class CSoundModifierStack : public CObject
{
CONSTRUCTABLE(CSoundModifierStack);
DESTRUCTABLE(CSoundModifierStack);
protected:
	CArray<CSoundModifier>	maModifiers;  //Sound | Modifier 0 | Modifier 1 | etc...
	Ptr<CSound>				mpSound;

public:
	void 					Init(Ptr<CSound> pSound = NULL);
	void					Class(void) override;
	void 					Free(void) override;

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void 					AddModifier(Ptr<CSoundModifier> pModifier);
	template<class M, typename ... Args>
	Ptr<M>					AddModifier(Args ... args);

	void					SetSound(Ptr<CSound> pSound);
	void					ApplyAll(void);  //The sound set is changed in place, so everything pointing to it sees the result.

	int						NumModifiers(void);
	Ptr<CSound>				GetSound(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, typename ... Args>
Ptr<M> CSoundModifierStack::AddModifier(Args ... args)
{
	Ptr<M>	pModifier;

	pModifier = OMalloc<M>(args ...);
	AddModifier(pModifier);
	return pModifier;
}


#endif // __SOUND_MODIFIER_STACK_H__

