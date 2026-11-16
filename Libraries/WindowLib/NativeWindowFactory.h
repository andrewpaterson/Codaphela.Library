/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __NATIVE_WINDOW_FACTORY_H__
#define __NATIVE_WINDOW_FACTORY_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"
#include "NativeFactory.h"
#include "NativeWindow.h"
#include "NativeCanvas.h"
#include "NativeClipboard.h"


class CWindow;
class CCanvas;
class CClipboard;


//Makes the host's side of windows, canvases and clipboards (CNativeWindow, CNativeCanvas, CNativeClipboard), allocated
//with the mallocator it's given.  It's an object so the object system frees it after everything that uses what it made:
//CWindow, CCanvas and CClipboard hold it (Nes), so it outlives them whatever order the app shuts down in, ObjectsKill
//included.  The app makes it with OMalloc after ObjectsInit and roots it (or a test holds it on the stack), and lets go
//of it rather than killing it.  It isn't saved.
class CNativeWindowFactory : public CObject
{
CONSTRUCTABLE(CNativeWindowFactory);
protected:
	CMallocator*	mpcMallocator;

public:
			void			Init(CMallocator* pcMallocator);  //Only from a factory's own Init, between its PreInit and PostInit.
			void			Class(void) override;
			void			Free(void) override;
			bool			Save(CObjectWriter* pcFile) override;
			bool			Load(CObjectReader* pcFile) override;

	virtual CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) =0;
			bool			DestroyNativeWindow(CNativeWindow* pcWindow);

	virtual CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) =0;
			bool			DestroyNativeCanvas(CNativeCanvas* pcCanvas);

	virtual CNativeClipboard*	CreateNativeClipboard(CClipboard* pcClipboard) =0;  //NULL if there is no clipboard.
			bool				DestroyNativeClipboard(CNativeClipboard* pcClipboard);

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


#endif // __NATIVE_WINDOW_FACTORY_H__

