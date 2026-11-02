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
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"
#include "NativeWindow.h"
#include "NativeCanvas.h"
#include "NativeClipboard.h"


class CWindow;
class CCanvas;
class CClipboard;
class CNativeWindowFactory : public CConstructable
{
CONSTRUCTABLE(CNativeWindowFactory);
friend class CWindowFactory;
protected:
	CMallocator*	mpcMallocator;

public:
			void			Init(CMallocator* pcMallocator);
			void			Kill(void);

	virtual CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) =0;
			bool			DestroyNativeWindow(CNativeWindow* pcWindow);

	virtual CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) =0;
			bool			DestroyNativeCanvas(CNativeCanvas* pcCanvas);

	virtual CNativeClipboard*	CreateNativeClipboard(CClipboard* pcClipboard) =0;  //NULL if there is no clipboard.
			bool			DestroyNativeClipboard(CNativeClipboard* pcClipboard);

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



#endif // __NATIVE_WINDOW_FACTORY_H__

