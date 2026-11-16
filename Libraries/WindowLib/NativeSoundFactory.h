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
#include "StandardLib/Object.h"
#include "NativeFactory.h"
#include "NativeSoundMixer.h"


//Makes the host's sound output.  It is separate from the native window factory so a window library doesn't depend on
//a sound library: the application makes one of each and gives both to its window.  Like the window factory it's an
//object: CSoundMixer holds it (Nes), so it's freed after every mixer it made, whatever order the app shuts down in.
//The app makes it with OMalloc after ObjectsInit and roots it (or a test holds it on the stack), and lets go of it
//rather than killing it.  It isn't saved.
class CNativeSoundFactory : public CObject
{
CONSTRUCTABLE(CNativeSoundFactory);
protected:
	CMallocator*	mpcMallocator;

public:
			void				Init(CMallocator* pcMallocator);
			void				Class(void) override;
			void				Free(void) override;
			bool				Save(CObjectWriter* pcFile) override;
			bool				Load(CObjectReader* pcFile) override;

	virtual CNativeSoundMixer*	CreateNativeSoundMixer(CSoundMixer* pcMixer) =0;
			bool				DestroyNativeSoundMixer(CNativeSoundMixer* pcNativeMixer);

protected:
	template<class Class>
	Class* NativeMalloc(void)
	{
		return NativeFactoryMalloc<Class>(mpcMallocator);
	}

	template<class Class>
	bool NativeFree(Class* pcInstance)
	{
		return NativeFactoryFree(mpcMallocator, pcInstance);
	}
};


#endif // __NATIVE_SOUND_FACTORY_H__

