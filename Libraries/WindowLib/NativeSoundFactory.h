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
#ifndef __NATIVE_SOUND_FACTORY_H__
#define __NATIVE_SOUND_FACTORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Mallocator.h"
#include "NativeSoundMixer.h"


//Makes the host's sound output.  It is separate from the native window factory so a window library doesn't depend on
//a sound library: the application makes one of each and gives both to its window.
class CNativeSoundFactory : public CConstructable
{
CONSTRUCTABLE(CNativeSoundFactory);
protected:
	CMallocator*	mpcMallocator;

public:
			void				Init(CMallocator* pcMallocator);
			void				Kill(void);

	virtual CNativeSoundMixer*	CreateNativeSoundMixer(CSoundMixer* pcMixer) =0;
			bool				DestroyNativeSoundMixer(CNativeSoundMixer* pcNativeMixer);

protected:
	template<class Class>
	Class* NativeMalloc(void)
	{
		Class* pcInstance;

		pcInstance = (Class*)mpcMallocator->Malloc(sizeof(Class));
		New<Class>(pcInstance);

		return pcInstance;
	}
};


#endif // __NATIVE_SOUND_FACTORY_H__

